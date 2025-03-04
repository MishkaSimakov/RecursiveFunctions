#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::visit_id_expression(IdExpr& node) {
  auto [scope, info] = qualified_name_lookup(current_scope_, node);
  if (info == nullptr) {
    scold_user(node, "Unknown identifier.");
  }

  if (!std::holds_alternative<TerminalSymbol>(info->data)) {
    scold_user(
        node,
        "Identifier must refer to variable, function or function parameter.");
  }

  node.type = std::get<TerminalSymbol>(info->data).type;

  return true;
}
}  // namespace Front
