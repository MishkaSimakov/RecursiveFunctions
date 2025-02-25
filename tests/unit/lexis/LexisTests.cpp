#include <gtest/gtest.h>

#include <vector>

#include "LexisTestCase.h"

using enum Lexis::TokenType::InternalEnum;

TEST_F(LexisTestCase, empty_string_test) { test_sequence({{"", WHITESPACE}}); }

TEST_F(LexisTestCase, simple_tests) {
  test_sequence({{"hello", IDENTIFIER},
                 {"     ", WHITESPACE},
                 {"// hello world!!!\n", WHITESPACE},
                 {"=", EQUAL},
                 {"5", NUMBER}});

  test_sequence({{"a", IDENTIFIER}, {" ", WHITESPACE}, {"b", IDENTIFIER}});

  test_sequence({{"// comment !!!", WHITESPACE}});
  test_sequence({{"// "
                  "comment !!!\n",
                  WHITESPACE},
                 {"identifier", IDENTIFIER},
                 {";", SEMICOLON},
                 {"// comment", WHITESPACE}});
}

TEST_F(LexisTestCase, test_number_token) {
  for (size_t i = 0; i < 100; ++i) {
    test_sequence({{std::to_string(i), NUMBER}});
  }

  std::string_view very_long =
      "12370837148274897328946815618973472308168356189658932714374";
  test_sequence({{very_long, NUMBER}});
}

TEST_F(LexisTestCase, test_identifier_token) {
  std::vector identifiers = {"a",   "b",  "c",    "a_a",      "a___", "_123",
                             "a1a", "a1", "a123", "_1_2_3_4", "ABC_", "_A"};

  for (std::string_view id : identifiers) {
    test_sequence({{id, IDENTIFIER}});
  }

  std::string_view very_long =
      "asldkfjwriqu923874213ADFVBADBDA____asdfhkasdhjfh123123_asadfASDF";
  test_sequence({{very_long, IDENTIFIER}});
}
