#include "SemanticAnalyzer.h"

namespace Front {

bool SemanticAnalyzer::visit_tuple_index_expression(TupleIndexExpr& node) {
  TupleType* left_ty = static_cast<TupleType*>(node.left->type);

  if (left_ty == nullptr) {
    scold_user(node, "type {:?} doesn't have tuple indexation",
               node.left->type);
  }

  size_t tuple_size = left_ty->elements.size();
  if (node.index < 0 || tuple_size <= node.index) {
    scold_user(node, "tuple index is out of range [0..{})", tuple_size);
  }

  node.value_category = node.left->value_category;
  node.type = left_ty->elements[node.index];

  return true;
}

}  // namespace Front
