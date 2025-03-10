#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::traverse_if_statement(IfStmt& node) {
  traverse(*node.condition);

  if (node.condition->type->get_kind() != Type::Kind::BOOL) {
    scold_user(*node.condition,
               "Condition of if statement must be of boolean type.");
  }

  {
    NestedScopeRAII scope_guard(current_scope_);
    traverse(*node.true_branch);
  }

  {
    NestedScopeRAII scope_guard(current_scope_);
    traverse(*node.false_branch);
  }

  return true;
}
}  // namespace Front
