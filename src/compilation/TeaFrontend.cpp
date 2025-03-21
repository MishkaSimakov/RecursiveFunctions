#include "TeaFrontend.h"

#include <deque>
#include <iostream>

#include "ast/ASTPrinter.h"
#include "compilation/semantics/SemanticAnalyzer.h"
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
  Lexis::LexicalAnalyzer lexical_analyzer(Constants::lexis_filepath);
  auto parser = Syntax::LRParser(Constants::grammar_filepath);

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

    if (has_syntax_errors) {
      continue;
    }

    // processing imports
    for (const auto& import_decl : module_context.ast_root->imports) {
      std::string_view import_name = module_context.get_string(import_decl->id);

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

void TeaFrontend::build_symbols_table() {
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
    for (const auto& dependency : current_module.dependencies) {
      if (!dependency.get().has_symbols_table) {
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
      auto analyzer = SemanticAnalyzer(context_, current_module);
      analyzer.analyze();
    } catch (SemanticAnalyzerException exception) {
      for (const auto& [position, error] : exception.errors) {
        context_.source_manager.add_annotation(position, error);
      }

      context_.source_manager.print_annotations(std::cout);
      throw;
    }
  }
}

int TeaFrontend::compile() {
  // Creating context for each module before building ast
  // this way we can store links to imported modules
  for (const auto& name : files_ | std::views::keys) {
    context_.add_module(name);
  }

  // For each module build ASTTree and store links to imported modules
  build_ast();

  // For each module build symbol table
  build_symbols_table();

  return 0;
}
}  // namespace Front
