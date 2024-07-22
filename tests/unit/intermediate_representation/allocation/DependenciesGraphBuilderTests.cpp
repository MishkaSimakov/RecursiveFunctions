#include <gtest/gtest.h>

#include <vector>

#include "DependenciesGraphBuilderTestCase.h"

TEST_F(DependenciesGraphBuilderTestCase, test_with_one_instruction) {
  BasicBlock block;

  // %2 = %0 + %1
  auto addition = std::make_unique<Addition>();
  addition->result_destination = Temporary{2};
  addition->left = Temporary{0};
  addition->right = Temporary{1};
  block.instructions.push_back(std::move(addition));

  auto return_instruction = std::make_unique<Return>();
  return_instruction->value = TemporaryOrConstant::temporary(2);
  block.instructions.push_back(std::move(return_instruction));

  Function function("main");
  function.arguments_count = 2;
  function.set_begin_block(std::move(block));

  auto graph = build(function);

  DependenciesListT dependencieses = {{0, 1}};
  test_graph(graph, dependencieses);
}

TEST_F(DependenciesGraphBuilderTestCase,
       test_functions_arguments_dependencies) {
  BasicBlock block;

  auto return_instruction = std::make_unique<Return>();
  return_instruction->value = TemporaryOrConstant::constant(0);
  block.instructions.push_back(std::move(return_instruction));

  Function function("main");
  function.arguments_count = 3;
  function.set_begin_block(std::move(block));

  auto graph = build(function);

  DependenciesListT dependencieses = {{0, 1}, {0, 2}, {1, 2}};
  test_graph(graph, dependencieses);
}

TEST_F(DependenciesGraphBuilderTestCase,
       test_return_value_expands_lifetime_of_temporary) {
  // main(%0):
  // %1 = %0
  // return %0

  // in this test %0 and %1 has overlaping lifetimes so there must be edge
  // between them in graph

  BasicBlock block;

  auto addition = std::make_unique<Move>();
  addition->result_destination = Temporary{1};
  addition->source = Temporary{0};
  block.instructions.push_back(std::move(addition));

  auto return_instruction = std::make_unique<Return>();
  return_instruction->value = TemporaryOrConstant::temporary(0);
  block.instructions.push_back(std::move(return_instruction));

  Function function("main");
  function.arguments_count = 1;
  function.set_begin_block(std::move(block));

  auto graph = build(function);

  DependenciesListT dependencieses = {{0, 1}};
  test_graph(graph, dependencieses);
}
