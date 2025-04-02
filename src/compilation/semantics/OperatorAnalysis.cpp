#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::visit_binary_operator(BinaryOperator& node) {
  convert_to_rvalue(node.left);
  convert_to_rvalue(node.right);

  auto left_type = node.left->type;
  auto right_type = node.right->type;

  if (node.is_arithmetic()) {
    if (!left_type->is_primitive() || !right_type->is_primitive()) {
      scold_user(node, "Artithmetic is allowed only for primitive types.");
    }

    PrimitiveType* left_ptype = static_cast<PrimitiveType*>(left_type);
    PrimitiveType* right_ptype = static_cast<PrimitiveType*>(right_type);

    if (left_ptype->width != right_ptype->width) {
      scold_user(node,
                 "Artithmetic is allowed only for types with same width.");
    }

    size_t width = left_ptype->width;

    if (left_type->get_kind() == Type::Kind::SIGNED_INT &&
        right_type->get_kind() == Type::Kind::SIGNED_INT) {
      node.type = types().add_primitive<SignedIntType>(width);
    } else if (left_type->get_kind() == Type::Kind::UNSIGNED_INT &&
               right_type->get_kind() == Type::Kind::UNSIGNED_INT) {
      node.type = types().add_primitive<UnsignedIntType>(width);
    } else {
      scold_user(node, "Incompatible type for arithmetic operator.");
    }
  } else if (node.is_inequality()) {
    if (!left_type->is_arithmetic() || !right_type->is_arithmetic()) {
      scold_user(node, "Incompatible operands types.");
    }

    node.type = types().add_primitive<BoolType>(8);
  } else if (node.is_equality()) {
    if (left_type != right_type) {
      scold_user(node, "Both operands of equality must be of same type.");
    }

    node.type = types().add_primitive<BoolType>(8);
  } else {
    scold_user(node, "Unimplemented binary operator.");
  }

  node.value_category = ValueCategory::RVALUE;

  return true;
}

bool SemanticAnalyzer::visit_unary_operator(UnaryOperator& node) {
  not_implemented();
  return true;
}
}  // namespace Front
