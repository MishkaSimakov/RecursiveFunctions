#pragma once

#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <type_traits>

#include "RecursiveFunctions.h"

using Compilation::CompileTreeBuilder, Compilation::CompileNode,
    Preprocessing::Preprocessor, Preprocessing::TextSource,
    RecursiveFunctionsSyntax::RuleIdentifiers;
using std::string;

class CompileTreeTestCase : public ::testing::Test {
 protected:
  constexpr static auto kSuccessor = "successor";
  constexpr static auto kArgmin = "argmin";

  CompileTreeTestCase() { Logger::disable_category(Logger::ALL); }

  template <typename... Args>
    requires(std::is_constructible_v<string, Args> && ...)
  static std::unique_ptr<CompileNode> get_tree(Args... program) {
    auto preprocessor = Preprocessor();
    preprocessor.add_source<TextSource>("main",
                                        vector<string>{std::move(program)...});
    string program_text = preprocessor.process();

    auto tokens = LexicalAnalyzer::get_tokens(program_text);

    auto syntax_root =
        SyntaxTreeBuilder::build(tokens, RecursiveFunctionsSyntax::GetSyntax(),
                                 RuleIdentifiers::PROGRAM);

    CompileTreeBuilder ct_builder;
    return ct_builder.build(*syntax_root);
  }

  template <typename U, typename T>
    requires std::is_base_of_v<CompileNode, T> && std::is_base_of_v<T, U>
  static U& node_cast(const std::unique_ptr<T>& node) {
    return dynamic_cast<U&>(*node);
  }
};
