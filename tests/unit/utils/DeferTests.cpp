#include <gtest/gtest.h>

#include "utils/Defer.h"

TEST(DeferTests, move_construct) {
  int counter = 0;

  {
    auto a = Defer([&counter] { ++counter; });

    auto b = std::move(a);
  }

  ASSERT_EQ(counter, 1);
}
