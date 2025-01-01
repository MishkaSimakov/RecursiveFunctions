#include "TeaFrontend.h"

#include <iostream>

#include "ast/ASTPrinter.h"
#include "compilation/ImportASTVisitor.h"
#include "compilation/TypeASTVisitor.h"
#include "lexis/LexicalAnalyzer.h"
#include "syntax/lr/LRParser.h"
#include "utils/Constants.h"

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
  // setup parser and lexical analyzer, load tables
  Lexis::LexicalAnalyzer lexical_analyzer(Constants::lexis_filepath,
                                          source_manager_);
  auto parser = Syntax::LRParser(Constants::grammar_filepath, source_manager_);

  // process each file separately
  for (auto& [name, path] : files) {
    SourceLocation begin = source_manager_.load(path);
    lexical_analyzer.set_location(begin);

    ASTBuildContext build_context(source_manager_, types_storage_);

    auto ast = parser.parse(lexical_analyzer);
    ImportASTVisitor(ast).traverse();

    std::cout << "Module " << name << ":\n";
    std::cout << "AST:\n";
    ASTPrinter(ast, std::cout, source_manager_).print();

    std::cout << "\n";
    std::cout << "Imports:\n";
    for (size_t id : ast.imports) {
      std::cout << ast.string_literals_table[id].quoteless_view() << std::endl;
    }
    std::cout << std::endl;

    modules_.emplace(name, ModuleCompileInfo{name, std::move(ast)});
  }

  for (auto& module : modules_ | std::views::values) {
    if (module.context.imports.empty()) {
      start_modules_.push_back(&module);
      continue;
    }

    for (size_t dependency : module.context.imports) {
      std::string_view dependency_name =
          module.context.string_literals_table[dependency].quoteless_view();
      auto dependency_module = &modules_.at(dependency_name);

      module.dependencies.push_back(dependency_module);
      dependency_module->next.push_back(&module);
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

  // actually do compilation
  // TypeASTVisitor(*module, modules_).traverse();

  module->is_processed = true;
  return true;
}

int TeaFrontend::compile(
    const std::unordered_map<std::string, std::filesystem::path>& files) {
  {
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
  }

  return 0;
}
