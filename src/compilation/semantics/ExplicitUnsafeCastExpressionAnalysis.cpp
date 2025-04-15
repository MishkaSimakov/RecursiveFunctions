#include "SemanticAnalyzer.h"

namespace Front {

bool is_unsafe_cast_allowed(Type* from, Type* to) {
  if (from == to) {
    return true;
  }

  if (from->is_arithmetic() && to->is_arithmetic()) {
    return true;
  }

  return false;
}

bool SemanticAnalyzer::visit_explicit_unsafe_cast_expression(
    ExplicitUnsafeCastExpr& node) {
  // unsafe cast is allowed between all arithmetic types
  Type* child_type = node.child->type;
  Type* result_type = node.type_node->value;

  if (!is_unsafe_cast_allowed(child_type, result_type)) {
    scold_user(node, "cast from {} to {} is forbidden", child_type,
               result_type);
  }

  convert_to_rvalue(node.child);

  node.type = result_type;
  node.value_category = ValueCategory::RVALUE;

  return true;
}

}  // namespace Front
