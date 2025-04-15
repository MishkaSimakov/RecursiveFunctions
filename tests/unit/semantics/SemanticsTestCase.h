#pragma once

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <lexis/LexicalAnalyzer.h>
#include <syntax/lr/LRParser.h>
#include <utils/Constants.h>

#include "compilation/GlobalContext.h"
#include "compilation/semantics/SemanticAnalyzer.h"
#include "sources/SourceManager.h"

using namespace Front;

class SemanticsTestCase : public ::testing::Test {
 private:
  std::unique_ptr<GlobalContext> context_;

 protected:
  const ModuleContext& module() { return context_->get_module("main"); }

  template <typename... Args>
  bool is_identifier_equal(const std::unique_ptr<IdExpr>& identifier,
                           const Args&... parts) {
    return is_identifier_equal(*identifier, parts...);
  }

  template <typename... Args>
  bool is_identifier_equal(const IdExpr& identifier, const Args&... parts) {
    std::vector<std::string_view> expected_parts{parts...};

    if (expected_parts.size() != identifier.id.parts.size()) {
      return false;
    }

    for (size_t i = 0; i < expected_parts.size(); ++i) {
      if (module().get_string(identifier.id.parts[i]) != expected_parts[i]) {
        return false;
      }
    }

    return true;
  }

  ModuleContext& analyze(std::string_view program) {
    // reset global context for each analyze call
    context_ = std::make_unique<GlobalContext>();

    Lexis::LexicalAnalyzer lexical_analyzer(
        Constants::GetRuntimeFilePath(Constants::lexis_relative_filepath));
    auto source_view = context_->source_manager.load_text(program);
    lexical_analyzer.set_source_view(source_view);

    auto& module = context_->add_module("main");

    Syntax::LRParser parser(
        Constants::GetRuntimeFilePath(Constants::grammar_relative_filepath));
    parser.parse(lexical_analyzer, module, source_view);

    SemanticAnalyzer analyzer(module);
    analyzer.analyze();

    return module;
  }

  auto analyze_function(std::string_view function_body) {
    std::string program = "main: () -> i64 = {";
    program += function_body;
    program += " return 0; }";

    ModuleContext& module = analyze(program);
    auto& main_symbol =
        module.root_scope->symbols.at(module.add_string("main"));

    auto& function_decl =
        dynamic_cast<FunctionDecl&>(*module.ast_root->declarations[0]);

    return std::make_pair(
        std::ref(*std::get<FunctionSymbolInfo>(main_symbol).subscope),
        std::ref(function_decl.body->statements));
  }
};
