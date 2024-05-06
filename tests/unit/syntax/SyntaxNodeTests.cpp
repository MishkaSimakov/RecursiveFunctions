#include <gtest/gtest.h>

#include <string>

#include "SyntaxTestCase.h"

using Preprocessing::Preprocessor, RecursiveFunctionsSyntax::RuleIdentifiers;
using std::string;

TEST_F(SyntaxTestCase, test_it_compares_nested_syntax_nodes) {
  auto first = make_tree(
      SyntaxNodeType::ROOT, "",
      make_tree(SyntaxNodeType::FUNCTION, kSuccessor,
                make_tree(SyntaxNodeType::FUNCTION, kSuccessor,
                          make_tree(SyntaxNodeType::CONSTANT, "123"))));

  auto second = make_tree(
      SyntaxNodeType::ROOT, "",
      make_tree(SyntaxNodeType::FUNCTION, kSuccessor,
                make_tree(SyntaxNodeType::FUNCTION, kSuccessor,
                          make_tree(SyntaxNodeType::CONSTANT, "123"))));

  ASSERT_TREE_EQ(first, second);

  auto third = make_tree(
      SyntaxNodeType::ROOT, "",
      make_tree(SyntaxNodeType::FUNCTION, kSuccessor,
                make_tree(SyntaxNodeType::FUNCTION, kSuccessor,
                          make_tree(SyntaxNodeType::CONSTANT, "321"))));

  ASSERT_TREE_NE(first, third);

  auto fourth = make_tree(
      SyntaxNodeType::ROOT, "",
      make_tree(SyntaxNodeType::FUNCTION, kSuccessor,
                make_tree(SyntaxNodeType::FUNCTION, kSuccessor,
                          make_tree(SyntaxNodeType::VARIABLE, "123"))));

  ASSERT_TREE_NE(first, fourth);
  ASSERT_TREE_NE(third, fourth);

  auto fifth = make_tree(
      SyntaxNodeType::ROOT, "",
      make_tree(SyntaxNodeType::FUNCTION, kSuccessor,
                make_tree(SyntaxNodeType::FUNCTION, kSuccessor,
                          make_tree(SyntaxNodeType::VARIABLE, "123"),
                          make_tree(SyntaxNodeType::VARIABLE, "123"))));

  ASSERT_TREE_NE(fourth, fifth);
}
