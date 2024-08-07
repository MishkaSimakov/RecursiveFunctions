#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const RecursiveFunctionDefinitionNode& node) {
  wrap_with_function(node.name, node.arguments_count, [&node, this] {
    auto* condition_block = result_;

    condition_block->instructions.push_back(std::make_unique<Branch>(Value(
        node.get_recursion_parameter_index(), ValueType::VIRTUAL_REGISTER)));

    recursion_parameter_temporary_ = get_next_temporary();

    auto zero_case = current_function_->add_block();
    result_ = zero_case;
    node.zero_case->accept(*this);

    auto general_case = current_function_->add_block();
    result_ = general_case;
    node.general_case->accept(*this);

    Subtraction subtraction(recursion_parameter_temporary_,
                            Value(node.get_recursion_parameter_index(),
                                  ValueType::VIRTUAL_REGISTER),
                            Value(1, ValueType::CONSTANT));

    general_case->instructions.push_front(
        std::make_unique<Subtraction>(subtraction));

    condition_block->children = {zero_case, general_case};
  });
}
}  // namespace IR
