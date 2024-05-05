#include <gtest/gtest.h>

#include <string>

#include "SyntaxTestCase.h"

using std::string;
using Syntax::NoRulesMatchedException;

#define ASSERT_SYNTAX_INCORRECT(program) \
  ASSERT_THROW({ get_tree(program);\
}, NoRulesMatchedException);

TEST_F(SyntaxTestCase, test_it_throws_when_unmatched_paren) {
  ASSERT_SYNTAX_INCORRECT("f(x, y))=1;");
  ASSERT_SYNTAX_INCORRECT("f(x, y))=1;");
  ASSERT_SYNTAX_INCORRECT("f(x, y)=successor(1));");
  ASSERT_SYNTAX_INCORRECT("f(x, y)=successor(1));");
  ASSERT_SYNTAX_INCORRECT("(f(x, y)=successor(1));");
}

TEST_F(SyntaxTestCase, test_it_throws_when_redundant_comma) {
  ASSERT_SYNTAX_INCORRECT("test(x, y, )=1;");
  ASSERT_SYNTAX_INCORRECT("test(x, y)=successor(1, );");
  ASSERT_SYNTAX_INCORRECT("test(x, y)=successor(, 1);");
  ASSERT_SYNTAX_INCORRECT("test(x, y)=successor(,);");
  ASSERT_SYNTAX_INCORRECT("test(x, y)=123, ;");
  ASSERT_SYNTAX_INCORRECT("test(123, );");
  ASSERT_SYNTAX_INCORRECT("test(, 123);");
}

TEST_F(SyntaxTestCase, test_it_throws_when_no_function_name) {
  ASSERT_SYNTAX_INCORRECT("(x, y)=1;");
}

TEST_F(SyntaxTestCase, test_it_throws_when_no_function_definition) {
  ASSERT_SYNTAX_INCORRECT("test(x, y)=;");
}

TEST_F(SyntaxTestCase, test_it_throws_when_no_arguments) {
  ASSERT_SYNTAX_INCORRECT("test=successor(1);");
}

TEST_F(SyntaxTestCase, test_it_throws_when_nonconstant_in_call) {
  ASSERT_SYNTAX_INCORRECT("test(x, 123);");
  ASSERT_SYNTAX_INCORRECT("test(f(x));");
  ASSERT_SYNTAX_INCORRECT("test(f(1), x);");
}

TEST_F(SyntaxTestCase, test_it_throws_when_recursive_parameter_incorrect) {
  ASSERT_SYNTAX_INCORRECT("f(x, 0, y) = 12;");
  ASSERT_SYNTAX_INCORRECT("f(x + 1, y) = 12;");
  ASSERT_SYNTAX_INCORRECT("f(0, y + 1) = 12;");
  ASSERT_SYNTAX_INCORRECT("f(0, 0) = 12;");
  ASSERT_SYNTAX_INCORRECT("f(x, y + 1) = g(y + 1);");
}

TEST_F(SyntaxTestCase, test_it_throws_when_missing_semicolon) {
  ASSERT_SYNTAX_INCORRECT("f(x) = x");

  // this is quite subtle:
  // preprocessor will not remove whitespaces that would concatenate identifiers
  // or constants => lexer will fail when meet whitespace
  ASSERT_THROW({ get_tree("f(x) = x f(x);"); }, Lexing::UnexpectedSymbolException);
  ASSERT_SYNTAX_INCORRECT("f(x) = x; f(x)");
  ASSERT_SYNTAX_INCORRECT("f(123)");
}
