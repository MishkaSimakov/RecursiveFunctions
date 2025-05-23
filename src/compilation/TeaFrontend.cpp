#include "TeaFrontend.h"

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>

#include <deque>
#include <iostream>

#include "ast/ASTPrinter.h"
#include "compilation/semantics/SemanticAnalyzer.h"
#include "ir/IRGenerator.h"
#include "lexis/LexicalAnalyzer.h"
#include "syntax/lr/LRParser.h"
#include "utils/Constants.h"

namespace Front {
enum class DFSState { UNVISITED, VISITING, VISITED };

std::vector<std::string_view> has_loops_recursive(
    const ModuleContext& current,
    std::unordered_map<std::string_view, DFSState>& colors) {
  DFSState& color = colors[current.name];
  if (color == DFSState::VISITING) {
    return {current.name};
  }
  if (color == DFSState::VISITED) {
    return {};
  }

  color = DFSState::VISITING;
  for (auto child : current.dependents) {
    auto result = has_loops_recursive(child, colors);

    if (!result.empty()) {
      result.push_back(current.name);
      return result;
    }
  }
  color = DFSState::VISITED;

  return {};
}

std::vector<std::string_view> TeaFrontend::find_loops() const {
  std::unordered_map<std::string_view, DFSState> colors;

  for (const auto& [name, module] : context_.get_modules()) {
    if (colors[name] == DFSState::VISITED) {
      continue;
    }

    auto result = has_loops_recursive(module, colors);
    if (!result.empty()) {
      return result;
    }
  }

  return {};
}

void TeaFrontend::build_ast() {
  auto& source_manager = context_.source_manager;
  bool has_syntax_errors = false;

  // setup parser and lexical analyzer and parser
  // loading of tables occurs only once
  Lexis::LexicalAnalyzer lexical_analyzer(
      Constants::GetRuntimeFilePath(Constants::lexis_relative_filepath));
  auto parser = Syntax::LRParser(
      Constants::GetRuntimeFilePath(Constants::grammar_relative_filepath));

  // build ASTTree for each file separately
  // TODO: this can be easily parallelized
  for (const auto& [name, path] : files_) {
    auto& module_context = context_.get_module(name);

    SourceView source_view = source_manager.load(path);
    lexical_analyzer.set_source_view(source_view);

    try {
      parser.parse(lexical_analyzer, module_context, source_view);
    } catch (Syntax::ParserException exception) {
      has_syntax_errors = true;

      for (const auto& [position, error] : exception.get_errors()) {
        source_manager.add_annotation(position, error);
      }

      source_manager.print_annotations(std::cout);
    }

    module_context.state = ModuleContext::ModuleState::AFTER_PARSER;

    if (has_syntax_errors) {
      continue;
    }

    // processing imports
    for (const auto& import_decl : module_context.ast_root->imports) {
      std::string_view import_name =
          module_context.get_string(import_decl->name);

      if (!context_.has_module(import_name)) {
        throw std::runtime_error(fmt::format(
            "Compile error. Unknown module {:?} (imported in module {:?}).",
            import_name, module_context.name));
      }

      ModuleContext& import_context = context_.get_module(import_name);

      module_context.dependencies.push_back(import_context);

      // TODO: this is bad for parallelization
      import_context.dependents.push_back(module_context);
    }
  }

  if (has_syntax_errors) {
    throw std::runtime_error("Syntax errors encountered in files.");
  }

  // check that there is no import loops
  auto loop = find_loops();
  if (!loop.empty()) {
    throw std::runtime_error(fmt::format(
        "Compile error. Found loop in imports: {}.", fmt::join(loop, " -> ")));
  }
}

void TeaFrontend::build_symbols_table_and_compile() {
  std::deque<std::string_view> queue;

  for (const auto& [name, module] : context_.get_modules()) {
    if (module.dependencies.empty()) {
      queue.push_back(name);
    }
  }

  while (!queue.empty()) {
    std::string_view current_name = queue.front();
    queue.pop_front();

    // check that all dependencies are already processed
    ModuleContext& current_module = context_.get_module(current_name);
    bool has_unprocessed_dependencies = false;
    for (const ModuleContext& dependency : current_module.dependencies) {
      if (dependency.state != ModuleContext::ModuleState::AFTER_IR_COMPILER) {
        has_unprocessed_dependencies = true;
        break;
      }
    }

    if (has_unprocessed_dependencies) {
      queue.push_back(current_name);
      continue;
    }

    // build symbols table for module
    try {
      auto analyzer = SemanticAnalyzer(current_module);
      analyzer.analyze();
    } catch (SemanticAnalyzerException exception) {
      for (const auto& [position, error] : exception.errors) {
        context_.source_manager.add_annotation(position, error);
      }

      context_.source_manager.print_annotations(std::cout);
      throw;
    }

    auto ir_compiler = IRGenerator(*llvm_context_, current_module);
    llvm_modules_.emplace_back(std::move(ir_compiler.compile()));

    current_module.state = ModuleContext::ModuleState::AFTER_IR_COMPILER;

    for (const ModuleContext& dependent : current_module.dependents) {
      if (dependent.state != ModuleContext::ModuleState::AFTER_IR_COMPILER) {
        queue.push_back(dependent.name);
      }
    }
  }
}
void TeaFrontend::emit_ast() const {
  std::ofstream ofs;

  std::ostream& out = [&]() -> std::ostream& {
    if (output_file_.empty()) {
      return std::cout;
    }

    ofs.open(output_file_);
    return ofs;
  }();

  for (auto& [name, module] : context_.get_modules()) {
    out << "Module: " << name << std::endl;
    ASTPrinter(module, out).print();
  }
}

void TeaFrontend::emit_ir(const llvm::Module& main_module) const {
  std::ofstream ofs;

  std::ostream& out = [&]() -> std::ostream& {
    if (output_file_.empty()) {
      return std::cout;
    }

    ofs.open(output_file_);
    return ofs;
  }();

  llvm::raw_os_ostream llvm_out(out);
  main_module.print(llvm_out, nullptr);
}

TeaFrontend::TeaFrontend(TeaFrontendConfiguration config)
    : llvm_context_(std::make_unique<llvm::LLVMContext>()),
      files_(std::move(config.sources)),
      output_file_(std::move(config.output_file)),
      emit_type_(config.emit_type) {}

int TeaFrontend::compile() {
  OSO_FIRE();

  // Creating context for each module before building ast
  // this way we can store links to imported modules
  for (const auto& name : files_ | std::views::keys) {
    context_.add_module(name);
  }

  // For each module build ASTTree and store links to imported modules
  build_ast();

  if (emit_type_ == EmitType::AST) {
    emit_ast();
    return 0;
  }

  // For each module build symbol table and compile it into llvm IR
  build_symbols_table_and_compile();

  // Link all llvm modules together
  auto main_module = llvm::Module("main", *llvm_context_);
  llvm::Linker linker(main_module);

  for (auto& module : llvm_modules_) {
    linker.linkInModule(std::move(module));
  }
  llvm_modules_.clear();

  // Write linked module into output
  emit_ir(main_module);

  return 0;
}
}  // namespace Front
