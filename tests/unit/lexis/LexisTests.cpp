#include <gtest/gtest.h>

#include <vector>

#include "RecursiveFunctions.h"

using std::unordered_map, std::vector;
using namespace Lexing;

TEST(LexisTests, test_basic) {
  unordered_map<string, vector<Token>> tests = {
      {"abc=cba;", vector{Token{TokenType::IDENTIFIER, "abc"},
                          Token{TokenType::OPERATOR, "="},
                          Token{TokenType::IDENTIFIER, "cba"},
                          Token{TokenType::SEMICOLON, ";"}}},
      {"f(x,y)=f(y,x);",
       vector{Token{TokenType::IDENTIFIER, "f"}, Token{TokenType::LPAREN, "("},
              Token{TokenType::IDENTIFIER, "x"}, Token{TokenType::COMMA, ","},
              Token{TokenType::IDENTIFIER, "y"}, Token{TokenType::RPAREN, ")"},
              Token{TokenType::OPERATOR, "="},
              Token{TokenType::IDENTIFIER, "f"}, Token{TokenType::LPAREN, "("},
              Token{TokenType::IDENTIFIER, "y"}, Token{TokenType::COMMA, ","},
              Token{TokenType::IDENTIFIER, "x"}, Token{TokenType::RPAREN, ")"},
              Token{TokenType::SEMICOLON, ";"}}},
      {"f()=argmin(*);",
       vector{Token{TokenType::IDENTIFIER, "f"}, Token{TokenType::LPAREN, "("},
              Token{TokenType::RPAREN, ")"}, Token{TokenType::OPERATOR, "="},
              Token{TokenType::IDENTIFIER, "argmin"},
              Token{TokenType::LPAREN, "("}, Token{TokenType::ASTERISK, "*"},
              Token{TokenType::RPAREN, ")"},
              Token{TokenType::SEMICOLON, ";"}}}};

  for (const auto& [string, result] : tests) {
    ASSERT_EQ(LexicalAnalyzer::get_tokens(string), result);
  }
}

TEST(LexisTests, test_it_parse_identifier_as_one_token) {
  vector scary_identifiers = {"helloworld",   "hello_world", "hell_o_world",
                              "test____test", "____",        "a_b_c_"};

  for (auto& identifier : scary_identifiers) {
    auto tokens = LexicalAnalyzer::get_tokens(identifier);

    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens.front().type, TokenType::IDENTIFIER);
    ASSERT_EQ(tokens.front().value, identifier);
  }
}

TEST(LexisTests, test_it_throws_when_incorrect_symbol_occured) {
  vector strings_with_incorrect_symbols = {"f(x, y)@", "&abc",
                                           "successor(абырвалг)", "😇🙂🙂🙂"};

  for (auto& incorrect : strings_with_incorrect_symbols) {
    ASSERT_THROW({ LexicalAnalyzer::get_tokens(incorrect); }, UnexpectedSymbolException);
  }
}

TEST(LexisTests, test_it_throws_when_symbol_must_not_repeat) {
  vector strings_with_repeated_symbol = {
      "f(x, y) == x;",         "f(x,, y) = z;", "f(x, y) = z;;", ";;",
      "f(x, y) = argmin(**);", "f((x, y)=x;"};

  for (auto& incorrect : strings_with_repeated_symbol) {
    ASSERT_THROW({ LexicalAnalyzer::get_tokens(incorrect); }, UnexpectedSymbolException);
  }
}


