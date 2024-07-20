#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const RecursiveFunctionDefinitionNode& node) {
  wrap_with_function(node.name, node.arguments_count, [&node, this] {
    auto* condition_block = result_;
    condition_block->end_value =
        TemporaryOrConstant::temporary(node.get_recursion_parameter_index());

    recursion_parameter_temporary_ = get_next_temporary();

    auto zero_case = current_function_->add_block();
    result_ = zero_case;
    node.zero_case->accept(*this);

    auto general_case = current_function_->add_block();
    result_ = general_case;
    node.general_case->accept(*this);

    zero_case->parents = {condition_block};
    general_case->parents = {condition_block};

    Subtraction subtraction;
    subtraction.result_destination = recursion_parameter_temporary_;
    subtraction.left = Temporary{node.get_recursion_parameter_index()};
    subtraction.right = TemporaryOrConstant::constant(1);

    general_case->instructions.push_front(
        std::make_unique<Subtraction>(subtraction));

    condition_block->children.first = zero_case;
    condition_block->children.second = general_case;
  });
}
}  // namespace IR
