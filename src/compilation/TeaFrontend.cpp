#include "TeaFrontend.h"

#include <iostream>

#include "ast/ASTPrinter.h"
#include "compilation/ScopePrinter.h"
#include "lexis/LexicalAnalyzer.h"
#include "syntax/lr/LRParser.h"
#include "types/TypesASTVisitor.h"
#include "utils/Constants.h"

namespace Front {
bool TeaFrontend::has_loops_recursive(
    const ModuleCompileInfo* current,
    std::unordered_map<const ModuleCompileInfo*, int>& colors) {
  // colors:
  // 0 - unvisited
  // 1 - in current DFS path
  // 2 - visited but not on path

  int& color = colors[current];
  if (color != 0) {
    return color == 2;
  }

  color = 1;
  for (auto child : current->next) {
    if (has_loops_recursive(child, colors)) {
      return true;
    }
  }
  color = 2;

  return false;
}

void TeaFrontend::build_ast_and_dependencies(
    const std::unordered_map<std::string, std::filesystem::path>& files) {
  auto& source_manager = context_.source_manager;

  // setup parser and lexical analyzer, load tables
  Lexis::LexicalAnalyzer lexical_analyzer(Constants::lexis_filepath);
  auto parser = Syntax::LRParser(Constants::grammar_filepath, context_);

  // process each file separately
  for (auto& [name, path] : files) {
    SourceView source_view = source_manager.load(path);
    lexical_analyzer.set_source_view(source_view);

    auto& module_context = context_.add_module(name);

    parser.parse(lexical_analyzer, module_context.id);

    compile_info_.emplace(module_context.id, ModuleCompileInfo{module_context});

    std::cout << "Module " << name << ":\n";
    std::cout << "AST:\n";
    ASTPrinter(context_, module_context, std::cout).print();

    std::cout << "\n";
    std::cout << "Imports:\n";
    for (StringId id : module_context.imports) {
      std::cout << context_.get_string(id) << std::endl;
    }
    std::cout << std::endl;
  }

  for (ModuleContext& module_context : context_.modules) {
    auto& compile_info = compile_info_.at(module_context.id);

    for (StringId import_id : module_context.imports) {
      size_t import_module_id =
          context_.module_names_mapping[context_.get_string(import_id)]->id;
      compile_info.dependencies.push_back(&compile_info_.at(import_module_id));
      compile_info_.at(import_module_id).next.push_back(&compile_info);
    }

    if (compile_info.dependencies.empty()) {
      start_modules_.push_back(&compile_info);
      continue;
    }
  }

  if (has_loops()) {
    throw std::runtime_error("Loops in imports are not allowed.");
  }
}

bool TeaFrontend::has_loops() const {
  std::unordered_map<const ModuleCompileInfo*, int> colors;

  for (auto module : start_modules_) {
    if (colors[module] == 2) {
      continue;
    }

    if (has_loops_recursive(module, colors)) {
      return true;
    }
  }

  return false;
}

bool TeaFrontend::try_compile_module(ModuleCompileInfo* module) {
  for (auto dependency : module->dependencies) {
    if (!dependency->is_processed) {
      return false;
    }
  }

  std::cout << "Compiling " << module->context.id << std::endl;
  // actually do compilation
  TypesASTVisitor(*module->context.ast_root, context_, module->context)
      .traverse();

  std::cout << "Scopes:" << std::endl;
  ScopePrinter(std::cout, context_, *module->context.root_scope).print();

  std::cout << std::endl;

  module->is_processed = true;
  return true;
}

int TeaFrontend::compile(
    const std::unordered_map<std::string, std::filesystem::path>& files) {
  build_ast_and_dependencies(files);

  std::vector<ModuleCompileInfo*> queue = start_modules_;
  while (!queue.empty()) {
    auto current = queue.back();
    queue.pop_back();

    if (!try_compile_module(current)) {
      continue;
    }

    for (auto next : current->next) {
      queue.push_back(next);
    }
  }

  return 0;
}
}  // namespace Front
