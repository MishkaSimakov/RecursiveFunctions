#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::traverse_compound_statement(CompoundStmt& node) {
  StringId subscope_name = context_.add_string(
      fmt::format("anonymous({})", anonymous_namespace_counter_++));
  Scope& subscope = current_scope_->add_child(subscope_name);

  NestedScopeRAII scope_guard(*this, subscope);

  for (auto& stmt : node.statements) {
    traverse(*stmt);
  }

  return true;
}
}  // namespace Front
