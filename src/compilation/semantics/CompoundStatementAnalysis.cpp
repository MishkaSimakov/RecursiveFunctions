#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::traverse_compound_statement(CompoundStmt& node) {
  NestedScopeRAII scope_guard(*this);

  for (auto& stmt : node.statements) {
    traverse(*stmt);
  }

  return true;
}
}  // namespace Front
