#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::visit_variable_declaration(VariableDecl& node) {
  auto type = node.type->value;
  auto [_, was_emplaced] = current_scope_->symbols.emplace(
      node.name, SymbolInfo::make_terminal(node, type));

  if (!was_emplaced) {
    auto name = context_.get_string(node.name);
    scold_user(node, fmt::format("Redefinition of variable {}", name));
  }

  return true;
}
}  // namespace Front
