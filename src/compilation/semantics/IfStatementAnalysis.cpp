#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::traverse_if_statement(IfStmt& node) {
  traverse(*node.condition);

  if (node.condition->type->get_kind() != Type::Kind::BOOL) {
    scold_user(*node.condition,
               "Condition of if statement must be of boolean type.");
  }

  traverse(*node.true_branch);
  traverse(*node.false_branch);

  return true;
}
}  // namespace Front
