#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::visit_binary_operator(BinaryOperator& node) {
  // TODO: make this better
  auto left_op_type = node.left->type;
  auto right_op_type = node.right->type;

  if (left_op_type->get_kind() != Type::Kind::INT ||
      right_op_type->get_kind() != Type::Kind::INT) {
    scold_user(node, "Both operands of binary operator must be of i64 type.");
  }

  node.type = types().add_primitive<IntType>();
  return true;
}

bool SemanticAnalyzer::visit_unary_operator(UnaryOperator& node) {
  scold_user(node, "Not implemented.");
  return true;
}
}  // namespace Front
