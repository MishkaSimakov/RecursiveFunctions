#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::before_compound_statement(CompoundStmt& node) {
  start_nested_scope();
  return true;
}

bool SemanticAnalyzer::after_compound_statement(CompoundStmt& node) {
  end_nested_scope();
  return true;
}
}  // namespace Front
