#pragma once

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "RecursiveFunctions.h"

using Compilation::CompileTreeBuilder, Compilation::CompileNode,
    Preprocessing::Preprocessor, Preprocessing::TextSource,
    RecursiveFunctionsSyntax::RuleIdentifiers;
using std::string;

#define SNTC_IS_SYNTAX_NODE(value)                            \
  static_assert(std::is_same_v<std::decay_t<decltype(value)>, \
                               std::unique_ptr<SyntaxNode>>);

#define ASSERT_TREE_EQ(lhs, rhs) \
  SNTC_IS_SYNTAX_NODE(lhs) SNTC_IS_SYNTAX_NODE(rhs) ASSERT_TRUE(*lhs == *rhs)

#define ASSERT_TREE_NE(lhs, rhs) \
  SNTC_IS_SYNTAX_NODE(lhs) SNTC_IS_SYNTAX_NODE(rhs) ASSERT_TRUE(*lhs != *rhs)

class SyntaxTestCase : public ::testing::Test {
 protected:
  constexpr static auto kSuccessor = "successor";
  constexpr static auto kArgmin = "argmin";

  SyntaxTestCase() { Logger::disable_category(Logger::ALL); }

  template <typename... Args>
    requires(std::is_constructible_v<string, Args> && ...)
  static std::unique_ptr<SyntaxNode> get_tree(Args... program) {
    auto preprocessor = Preprocessor();
    preprocessor.add_source<TextSource>("main",
                                        vector<string>{std::move(program)...});
    string program_text = preprocessor.process();

    auto tokens = LexicalAnalyzer::get_tokens(program_text);

    return SyntaxTreeBuilder::build(tokens,
                                    RecursiveFunctionsSyntax::GetSyntax(),
                                    RuleIdentifiers::PROGRAM);
  }

  template <typename... Args>
    requires(std::is_same_v<Args, std::unique_ptr<SyntaxNode>> && ...)
  static auto make_tree(SyntaxNodeType type, string value, Args&&... children) {
    auto node = std::make_unique<SyntaxNode>(type, std::move(value));
    (node->children.emplace_back(std::move(children)), ...);

    return node;
  }
};
