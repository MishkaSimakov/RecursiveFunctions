#include <gtest/gtest.h>

#include <string>

#include "SyntaxTestCase.h"

using Preprocessing::Preprocessor, RecursiveFunctionsSyntax::RuleIdentifiers;
using std::string;

TEST_F(SyntaxTestCase, test_it_parse_simple_call) {
  auto tree = get_tree("successor(successor(123));");

  auto expected = make_tree(
      SyntaxNodeType::ROOT, "",
      make_tree(SyntaxNodeType::FUNCTION, kSuccessor,
                make_tree(SyntaxNodeType::FUNCTION, kSuccessor,
                          make_tree(SyntaxNodeType::CONSTANT, "123"))));

  ASSERT_TREE_EQ(tree, expected);
}

TEST_F(SyntaxTestCase, test_it_parse_simple_function_definition) {
  auto tree = get_tree("f(x)=successor(x);");

  auto expected =
      make_tree(SyntaxNodeType::ROOT, "",
                make_tree(SyntaxNodeType::ASSIGNMENT, "",
                          make_tree(SyntaxNodeType::FUNCTION, "f",
                                    make_tree(SyntaxNodeType::VARIABLE, "x")),
                          make_tree(SyntaxNodeType::FUNCTION, kSuccessor,
                                    make_tree(SyntaxNodeType::VARIABLE, "x"))));

  ASSERT_TREE_EQ(tree, expected);
}

TEST_F(SyntaxTestCase, test_it_parse_argmin_expression) {
  auto tree = get_tree("f(x)=argmin(g(x, *));");

  auto expected = make_tree(
      SyntaxNodeType::ROOT, "",
      make_tree(
          SyntaxNodeType::ASSIGNMENT, "",
          make_tree(SyntaxNodeType::FUNCTION, "f",
                    make_tree(SyntaxNodeType::VARIABLE, "x")),
          make_tree(SyntaxNodeType::FUNCTION, kArgmin,
                    make_tree(SyntaxNodeType::FUNCTION, "g",
                              make_tree(SyntaxNodeType::VARIABLE, "x"),
                              make_tree(SyntaxNodeType::ASTERISK, "*")))));

  ASSERT_TREE_EQ(tree, expected);
}

TEST_F(SyntaxTestCase, test_is_parse_recursive_function_zero_case) {
  auto tree = get_tree("f(x, 0)=123;");

  auto expected = make_tree(
      SyntaxNodeType::ROOT, "",
      make_tree(SyntaxNodeType::ASSIGNMENT, "",
                make_tree(SyntaxNodeType::FUNCTION, "f",
                          make_tree(SyntaxNodeType::VARIABLE, "x"),
                          make_tree(SyntaxNodeType::RECURSION_PARAMETER, "0")),
                make_tree(SyntaxNodeType::CONSTANT, "123")));

  ASSERT_TREE_EQ(tree, expected);
}

TEST_F(SyntaxTestCase, test_is_parse_recursive_function_general_case) {
  auto tree = get_tree("f(x, y+1)=test(y);");

  auto expected = make_tree(
      SyntaxNodeType::ROOT, "",
      make_tree(SyntaxNodeType::ASSIGNMENT, "",
                make_tree(SyntaxNodeType::FUNCTION, "f",
                          make_tree(SyntaxNodeType::VARIABLE, "x"),
                          make_tree(SyntaxNodeType::RECURSION_PARAMETER, "y")),
                make_tree(SyntaxNodeType::FUNCTION, "test",
                          make_tree(SyntaxNodeType::VARIABLE, "y"))));

  ASSERT_TREE_EQ(tree, expected);
}

TEST_F(SyntaxTestCase, test_it_parse_multiple_statements) {
  auto tree = get_tree("f(x)=y;", "g(y)=x;", "f(123);");

  auto expected =
      make_tree(SyntaxNodeType::ROOT, "",
                make_tree(SyntaxNodeType::ASSIGNMENT, "",
                          make_tree(SyntaxNodeType::FUNCTION, "f",
                                    make_tree(SyntaxNodeType::VARIABLE, "x")),
                          make_tree(SyntaxNodeType::VARIABLE, "y")),
                make_tree(SyntaxNodeType::ASSIGNMENT, "",
                          make_tree(SyntaxNodeType::FUNCTION, "g",
                                    make_tree(SyntaxNodeType::VARIABLE, "y")),
                          make_tree(SyntaxNodeType::VARIABLE, "x")),
                make_tree(SyntaxNodeType::FUNCTION, "f",
                          make_tree(SyntaxNodeType::CONSTANT, "123")));

  ASSERT_TREE_EQ(tree, expected);
}
