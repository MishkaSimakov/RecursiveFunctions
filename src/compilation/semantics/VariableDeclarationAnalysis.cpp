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
  VariableSymbolInfo& var_info = std::get<VariableSymbolInfo>(info);

  // if we are inside function, then add variable to local variables list
  SymbolInfo* parent = current_scope_->get_parent_symbol();
  if (parent != nullptr) {
    if (auto* fun = std::get_if<FunctionSymbolInfo>(parent)) {
      fun->local_variables.push_back(var_info);
    } else if (auto* cls = std::get_if<ClassSymbolInfo>(parent)) {
      // TODO: check for members with same name
      cls->type->members.emplace_back(node.name, node.type->value);
    }
  }

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
