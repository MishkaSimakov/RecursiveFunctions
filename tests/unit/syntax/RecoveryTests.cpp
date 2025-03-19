// Good compiler must be able to recover from syntax and lexis errors
// To test this ability simple cases (where compiler definitely MUST be able
// to recover) are collected here to determine whether my compiler is good or
// not

#include <gtest/gtest.h>

#include <vector>

#include "SyntaxTestCase.h"

using Syntax::ParserException;

// override Google's EXPECT_THROW in this tests
#define MY_ASSERT_THROW(expression, exception)                     \
  try {                                                         \
    expression;                                                 \
    FAIL() << "Expression: " #expression "should have thrown."; \
  } catch (exception)

TEST_F(SyntaxTestCase, test_it_recovers_from_errors_in_braces_block) {
  {
    auto program = "f:()->void={error!} g:()->void={another!}";
    MY_ASSERT_THROW(parse(program), ParserException exception) {
      auto errors = exception.get_errors();

      ASSERT_EQ(errors.size(), 2);
      ASSERT_SOURCE_RANGE(errors[0].first, 17, 18);
      ASSERT_SOURCE_RANGE(errors[1].first, 39, 40);
    }
  }

  {
    auto program = "f:()->void={ { first! } call(); { second! } third! }";
    MY_ASSERT_THROW(parse(program), ParserException exception) {
      auto errors = exception.get_errors();

      ASSERT_EQ(errors.size(), 3);
      ASSERT_SOURCE_RANGE(errors[0].first, 20, 21);
      ASSERT_SOURCE_RANGE(errors[1].first, 40, 41);
      ASSERT_SOURCE_RANGE(errors[2].first, 49, 50);
    }
  }

  {
    auto program = "f:()->void={ { first } call(); { second! } }";
    MY_ASSERT_THROW(parse(program), ParserException exception) {
      auto errors = exception.get_errors();

      ASSERT_EQ(errors.size(), 2);
      ASSERT_SOURCE_RANGE(errors[0].first, 21, 22);
      ASSERT_SOURCE_RANGE(errors[1].first, 39, 40);
    }
  }

  {
    auto program = "math: namespace = { sqrt: (value: i64) -> i64 = { oops! } an_error! } func: () -> void = { error }";

    MY_ASSERT_THROW(parse(program), ParserException exception) {
      auto errors = exception.get_errors();

      ASSERT_EQ(errors.size(), 3);
      ASSERT_SOURCE_RANGE(errors[0].first, 54, 55);
      ASSERT_SOURCE_RANGE(errors[1].first, 66, 67);
      ASSERT_SOURCE_RANGE(errors[2].first, 97, 98);
    }
  }

  {
    auto program = "f: () -> void = { error {} { {} } } g: () -> void = { another! }";

    MY_ASSERT_THROW(parse(program), ParserException exception) {
      auto errors = exception.get_errors();

      ASSERT_EQ(errors.size(), 2);
      ASSERT_SOURCE_RANGE(errors[0].first, 24, 25);
      ASSERT_SOURCE_RANGE(errors[1].first, 61, 62);
    }
  }
}

TEST_F(SyntaxTestCase, test_it_recovers_from_errors_in_semicolons_sequence) {
  {
    auto program = "f: () -> void = { error!; func(); another!; }";
    MY_ASSERT_THROW(parse(program), ParserException exception) {
      auto errors = exception.get_errors();

      ASSERT_EQ(errors.size(), 2);
      ASSERT_SOURCE_RANGE(errors[0].first, 23, 24);
      ASSERT_SOURCE_RANGE(errors[1].first, 41, 42);
    }
  }
}
