#include <iostream>

#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::visit_variable_declaration(VariableDecl& node) {
  if (current_scope_->has_symbol(node.name)) {
    auto name = context_.get_string(node.name);
    scold_user(node, fmt::format("Redefinition of variable {}", name));
  }

  auto type = node.type->value;
  SymbolInfo& info = current_scope_->add_variable(node.name, node, type);
  current_scope_->add_local_variable(info.as<VariableSymbolInfo>());

  if (node.initializer != nullptr) {
    if (node.initializer->type != node.type->value) {
      scold_user(node,
                 "Type of variable initializer must be same as variable type.");
    }

    as_initializer(node.initializer);
  }

  add_to_exported_if_necessary(info);

  return true;
}
}  // namespace Front
