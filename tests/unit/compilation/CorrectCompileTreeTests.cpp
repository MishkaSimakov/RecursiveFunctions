#include <gtest/gtest.h>

#include "CompileTreeTestCase.h"

using namespace Compilation;

TEST_F(CompileTreeTestCase, test_it_build_simple_compile_tree) {
  auto tree = get_tree("f(x, y) = successor(x); f(1, 2);");

  const auto& func_definition = get_function(tree, "f");

  ASSERT_EQ(func_definition.arguments_count, 2);

  const auto& body = node_cast<FunctionDefinitionNode>(func_definition).body;
  const auto& successor = node_cast<FunctionCallNode>(body);

  ASSERT_EQ(get_function(tree, successor.index).name, "successor");
  ASSERT_EQ(node_cast<VariableNode>(successor.arguments[0]).index, 1);
  ASSERT_EQ(successor.arguments.size(), 1);
}
