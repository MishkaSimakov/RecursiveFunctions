#include <gtest/gtest.h>

#include "CompileTreeTestCase.h"

using namespace Compilation;

// TODO: test later
TEST_F(CompileTreeTestCase, test_it_throws_when_argmin_is_used_inside_argmin) {
  GTEST_SKIP() << "This test may not work correctly now, must be tested later";

  ASSERT_ANY_THROW(get_tree("f(x) = argmin(argmin(*)); f(0);"));
  ASSERT_ANY_THROW(
      get_tree("f(x) = successor(argmin(successor(argmin(*)))); f(1);"));
  ASSERT_ANY_THROW(get_tree(
      "g(x, y) = 123; f(x) = successor(argmin(g(argmin(*), *))); f(1);"));
}
