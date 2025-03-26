#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::visit_id_expression(IdExpr& node) {
  SymbolInfo* info = name_lookup(current_scope_, node.id);
  if (info == nullptr) {
    scold_user(node, "Unknown identifier.");
  }

  if (info->is_variable()) {
    node.type = std::get<VariableSymbolInfo>(*info).type;
  } else if (info->is_function()) {
    node.type = std::get<FunctionSymbolInfo>(*info).type;
  } else {
    scold_user(
        node,
        "Identifier must refer to variable, function or function parameter.");
  }

  node.value_category = ValueCategory::LVALUE;
  context_.symbols_info.emplace(&node, *info);

  return true;
}
}  // namespace Front
