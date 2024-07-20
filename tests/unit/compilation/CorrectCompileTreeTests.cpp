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

  //
  const auto& call = get_call(tree);
  ASSERT_EQ(get_function(tree, call.index).name, "f");
  ASSERT_EQ(call.arguments.size(), 2);
  ASSERT_EQ(node_cast<ConstantNode>(call.arguments[0]).value, 1);
  ASSERT_EQ(node_cast<ConstantNode>(call.arguments[1]).value, 2);
}

TEST_F(CompileTreeTestCase, test_it_build_argmin_call) {
  auto tree = get_tree("f(x, y)=x; argmin(f(5, *));");

  const auto& call = get_call<ArgminCallNode>(tree);
  const auto& wrapped = node_cast<FunctionCallNode>(call.wrapped_call);

  ASSERT_EQ(get_function(tree, wrapped.index).name, "f");
  ASSERT_EQ(wrapped.arguments.size(), 2);
  ASSERT_EQ(node_cast<ConstantNode>(wrapped.arguments[0]).value, 5);
  ASSERT_NO_THROW(node_cast<AsteriskNode>(wrapped.arguments[1]));
}

TEST_F(CompileTreeTestCase, test_it_build_recursive_function_with_prev_value) {
  auto tree =
      get_tree("add(x, 0) = x; add(x, y + 1) = successor(add); add(1, 2);");

  const auto& add_def =
      get_function<RecursiveFunctionDefinitionNode>(tree, "add");

  ASSERT_EQ(add_def.name, "add");
  ASSERT_EQ(add_def.arguments_count, 2);
  ASSERT_TRUE(add_def.use_previous_value);

  const auto& zero_case = node_cast<VariableNode>(add_def.zero_case);
  ASSERT_EQ(zero_case.index, 1);

  const auto& general_case = node_cast<FunctionCallNode>(add_def.general_case);

  ASSERT_EQ(get_function(tree, general_case.index).name, "successor");
  ASSERT_EQ(general_case.arguments.size(), 1);

  node_cast<SelfCallNode>(general_case.arguments[0]);
}

TEST_F(CompileTreeTestCase,
       test_it_build_recursive_function_without_prev_value) {
  auto tree = get_tree("not(0) = 1; not(x + 1) = 0; successor(1);");

  const auto& not_def =
      get_function<RecursiveFunctionDefinitionNode>(tree, "not");

  ASSERT_EQ(not_def.name, "not");
  ASSERT_EQ(not_def.arguments_count, 1);
  ASSERT_FALSE(not_def.use_previous_value);

  const auto& zero_case = node_cast<ConstantNode>(not_def.zero_case);
  ASSERT_EQ(zero_case.value, 1);

  const auto& general_case = node_cast<ConstantNode>(not_def.general_case);
  ASSERT_EQ(general_case.value, 0);
}

TEST_F(CompileTreeTestCase,
       test_it_build_recursive_function_with_recursive_parameter) {
  auto tree = get_tree("pred(0) = 0; pred(x + 1) = x; successor(1);");

  const auto& pred_def =
      get_function<RecursiveFunctionDefinitionNode>(tree, "pred");

  ASSERT_EQ(pred_def.name, "pred");
  ASSERT_EQ(pred_def.arguments_count, 1);
  ASSERT_FALSE(pred_def.use_previous_value);

  const auto& zero_case = node_cast<ConstantNode>(pred_def.zero_case);
  ASSERT_EQ(zero_case.value, 0);

  node_cast<RecursionParameterNode>(pred_def.general_case);
}
