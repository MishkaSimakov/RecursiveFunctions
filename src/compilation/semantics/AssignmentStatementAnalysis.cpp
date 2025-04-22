#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::visit_assignment_statement(AssignmentStmt& node) {
  // left part of assignment must be lvalue
  if (node.left->value_category != ValueCategory::LVALUE) {
    scold_user(*node.left, "left part of assignment must be lvalue");
  }

  if (node.left->type != node.right->type) {
    scold_user(node,
               "assignment must have same type on both sides: {:?} != {:?}",
               node.left->type, node.right->type);
  }

  // right part of assignment must be rvalue
  // if it is not then lvalue-to-rvalue-conversion is applied
  convert_to_rvalue(node.right);

  return true;
}
}  // namespace Front
