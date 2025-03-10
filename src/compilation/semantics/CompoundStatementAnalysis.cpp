#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::traverse_compound_statement(CompoundStmt& node) {
  NestedScopeRAII scope_guard(current_scope_);

  for (auto& stmt : node.statements) {
    traverse(*stmt);
  }

  return true;
}
}  // namespace Front
