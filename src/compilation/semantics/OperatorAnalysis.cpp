#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::visit_binary_operator(BinaryOperator& node) {
  // TODO: make this better
  auto left_op_type = node.left->type;
  auto right_op_type = node.right->type;

  if (node.is_arithmetic()) {
    if (left_op_type->get_kind() != Type::Kind::INT ||
        right_op_type->get_kind() != Type::Kind::INT) {
      scold_user(node,
                 "Both operands of arithmetic operator must be of i64 type.");
    }

    node.type = types().add_primitive<IntType>();
  } else if (node.is_inequality()) {
    if (left_op_type->get_kind() != Type::Kind::INT ||
        right_op_type->get_kind() != Type::Kind::INT) {
      scold_user(node,
                 "Both operands of inequality operator must be of i64 type.");
    }

    node.type = types().add_primitive<BoolType>();
  } else if (node.is_equality()) {
    if (left_op_type->get_kind() == Type::Kind::VOID ||
        right_op_type->get_kind() == Type::Kind::VOID) {
      scold_user(node, "Both operands of binary operator must be non-void.");
    }

    if (left_op_type != right_op_type) {
      scold_user(node,
                 "Both operands of binary operator must be of same type.");
    }

    node.type = types().add_primitive<BoolType>();
  } else {
    scold_user(node, "Unimplemented binary operator.");
  }

  return true;
}

bool SemanticAnalyzer::visit_unary_operator(UnaryOperator& node) {
  scold_user(node, "Not implemented.");
  return true;
}
}  // namespace Front
