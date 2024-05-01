#include <gtest/gtest.h>

#include <vector>

#include "RecursiveFunctions.h"

using namespace Lexing;

TEST(LexisTests, test_basic) {
  std::unordered_map<string, std::vector<Token>> tests = {
      {"abc=cba;", std::vector{Token{TokenType::IDENTIFIER, "abc"},
                               Token{TokenType::OPERATOR, "="},
                               Token{TokenType::IDENTIFIER, "cba"},
                               Token{TokenType::SEMICOLON, ";"}}},
      {"f(x,y)=f(y,x);",
       std::vector{
           Token{TokenType::IDENTIFIER, "f"}, Token{TokenType::LPAREN, "("},
           Token{TokenType::IDENTIFIER, "x"}, Token{TokenType::COMMA, ","},
           Token{TokenType::IDENTIFIER, "y"}, Token{TokenType::RPAREN, ")"},
           Token{TokenType::OPERATOR, "="}, Token{TokenType::IDENTIFIER, "f"},
           Token{TokenType::LPAREN, "("}, Token{TokenType::IDENTIFIER, "y"},
           Token{TokenType::COMMA, ","}, Token{TokenType::IDENTIFIER, "x"},
           Token{TokenType::RPAREN, ")"}, Token{TokenType::SEMICOLON, ";"}}}};

  for (const auto& [string, result] : tests) {
    ASSERT_EQ(LexicalAnalyzer::get_tokens(string), result);
  }
}