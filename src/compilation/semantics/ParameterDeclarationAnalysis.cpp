#include "SemanticAnalyzer.h"

namespace Front {
// TODO: maybe merge somehow with visit_variable_declaration
bool SemanticAnalyzer::visit_parameter_declaration(ParameterDecl& node) {
  auto type = node.type->value;
  auto [itr, was_emplaced] = current_scope_->symbols.emplace(
      node.id, SymbolInfo::make_terminal(node, type));

  if (!was_emplaced) {
    auto name = context_.get_string(node.id);
    scold_user(node, fmt::format("Redefinition of parameter {}", name));
  }

  return true;
}
}  // namespace Front
