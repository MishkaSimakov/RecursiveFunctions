#include <gtest/gtest.h>

#include <vector>

#include "LexisTestCase.h"

using enum Lexis::TokenType::InternalEnum;

TEST_F(LexisTestCase, empty_string_test) { test_sequence({{"", END}}); }

TEST_F(LexisTestCase, simple_tests) {
  test_sequence({{"hello", IDENTIFIER},
                 {"     ", WHITESPACE},
                 {"// hello world!!!\n", WHITESPACE},
                 {"=", EQUAL},
                 {"5", NUMBER}});

  test_sequence({{"a", IDENTIFIER}, {" ", WHITESPACE}, {"b", IDENTIFIER}});

  test_sequence({{"// comment !!!", WHITESPACE}});
}
