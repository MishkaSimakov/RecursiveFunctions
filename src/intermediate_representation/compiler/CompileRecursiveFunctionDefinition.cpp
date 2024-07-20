#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const RecursiveFunctionDefinitionNode& node) {
  current_temporary_index_ = node.arguments_count;

  auto zero_case = compile_node(node.zero_case);
  auto general_case = compile_node(node.general_case);

  auto condition_block = std::make_shared<BasicBlock>();
  condition_block->end_value =
      TemporaryOrConstant::temporary(node.get_recursion_parameter_index());

  zero_case.parents = {condition_block};
  general_case.parents = {condition_block};

  recursion_parameter_temporary_ = get_next_temporary();
  Subtraction subtraction;
  subtraction.result_destination = recursion_parameter_temporary_;
  subtraction.left = Temporary{node.get_recursion_parameter_index()};
  subtraction.right = TemporaryOrConstant::constant(1);

  general_case.instructions.push_front(
      std::make_unique<Subtraction>(subtraction));

  condition_block->children.first =
      std::make_shared<BasicBlock>(std::move(zero_case));
  condition_block->children.second =
      std::make_shared<BasicBlock>(std::move(general_case));

  program_.functions.emplace_back(node.name, std::move(condition_block));
}
}  // namespace IR
