#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::visit_assignment_statement(AssignmentStmt& node) {
  // left part of assignment must be lvalue
  if (node.left->value_category != ValueCategory::LVALUE) {
    scold_user(*node.left, "Left part of assignment must be lvalue.");
  }

  // right part of assignment must be rvalue
  // if it is not then lvalue-to-rvalue-conversion is applied
  convert_to_rvalue(node.right);

  return true;
}
}  // namespace Front
