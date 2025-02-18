#include "IRASTVisitor.h"

bool Front::IRASTVisitor::traverse_binary_operator(
    const BinaryOperator& value) {
  traverse(*value.left);
  IR::Value* left_operand = result_location_;

  traverse(*value.right);
  IR::Value* right_operand = result_location_;

  IR::Value* result_temp =
      current_function_->add_temporary(map_type(value.type));
  current_basic_block_->append_instruction<IR::Addition>(
      result_temp, left_operand, right_operand);

  result_location_ = result_temp;

  return true;
}
