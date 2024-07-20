#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const RecursiveFunctionDefinitionNode& node) {
  current_temporary_index_ = 0;

  auto zero_case = compile_node(node.zero_case);
  auto general_case = compile_node(node.general_case);

  auto condition_block = std::make_shared<BasicBlock>();
  condition_block->end_value =
      TemporaryOrConstant::temporary(node.arguments_count - 1);

  zero_case.parents = {condition_block};
  general_case.parents = {condition_block};

  condition_block->children.first =
      std::make_shared<BasicBlock>(std::move(zero_case));
  condition_block->children.second =
      std::make_shared<BasicBlock>(std::move(general_case));

  program_.functions.emplace_back(node.name, std::move(condition_block));
}
}  // namespace IR
