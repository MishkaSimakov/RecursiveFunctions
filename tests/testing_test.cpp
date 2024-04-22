#include <gtest/gtest.h>

#include "RecursiveFunctions.h"

TEST(HelloTest, BasicAssertions) {
  string program_text = "f(x)=x;f(10);";

  auto tokens = LexicalAnalyzer::get_tokens(program_text);

  ASSERT_NE(tokens.size(), 0);
}