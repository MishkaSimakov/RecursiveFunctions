#include <gtest/gtest.h>

#include "CompileTreeTestCase.h"

using namespace Compilation;

TEST_F(CompileTreeTestCase, test_it_build_simple_compile_tree) {
  auto tree = get_tree("f(x, y) = successor(x);");

  ProgramNode& node = node_cast<ProgramNode>(tree);
}
