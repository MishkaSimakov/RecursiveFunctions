#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const RecursiveFunctionDefinitionNode& node) {
  wrap_with_function(node.name, node.arguments_count, [&node, this] {
    auto zero_case = compile_node(node.zero_case);
    auto general_case = compile_node(node.general_case);

    BasicBlock condition_block;
    condition_block.end_value =
        TemporaryOrConstant::temporary(node.get_recursion_parameter_index());

    auto* condition_block_ptr =
        current_function_->set_begin_block(std::move(condition_block));

    zero_case.parents = {condition_block_ptr};
    general_case.parents = {condition_block_ptr};

    recursion_parameter_temporary_ = get_next_temporary();
    Subtraction subtraction;
    subtraction.result_destination = recursion_parameter_temporary_;
    subtraction.left = Temporary{node.get_recursion_parameter_index()};
    subtraction.right = TemporaryOrConstant::constant(1);

    general_case.instructions.push_front(
        std::make_unique<Subtraction>(subtraction));

    condition_block_ptr->children.first =
        current_function_->add_block(std::move(zero_case));
    condition_block_ptr->children.second =
      current_function_->add_block(std::move(general_case));
  });
}
}  // namespace IR
