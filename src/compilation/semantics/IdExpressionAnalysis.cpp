#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::visit_id_expression(IdExpr& node) {
  auto [scope, info] = qualified_name_lookup(current_scope_, node);
  if (info == nullptr) {
    scold_user(node, "Unknown identifier.");
  }

  if (info->is_variable()) {
    node.type = std::get<VariableSymbol>(info->data).type;
  } else if (info->is_function()) {
    node.type = std::get<FunctionSymbol>(info->data).type;
  } else {
    scold_user(
        node,
        "Identifier must refer to variable, function or function parameter.");
  }

  context_.symbols_info[&node] = info;

  return true;
}
}  // namespace Front
