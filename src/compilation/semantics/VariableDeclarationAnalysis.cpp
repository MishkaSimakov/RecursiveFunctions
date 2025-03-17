#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::visit_variable_declaration(VariableDecl& node) {
  if (current_scope_->has_symbol(node.name)) {
    auto name = context_.get_string(node.name);
    scold_user(node, fmt::format("Redefinition of variable {}", name));
  }

  auto type = node.type->value;
  current_scope_->add_variable(node.name, node, type);

  if (node.initializer != nullptr && node.initializer->type != node.type->value) {
    scold_user(node, "Type of variable initializer must be same as variable type.");
  }

  return true;
}
}  // namespace Front
