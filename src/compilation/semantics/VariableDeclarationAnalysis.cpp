#include <iostream>

#include "SemanticAnalyzer.h"

namespace Front {

void SemanticAnalyzer::check_for_name_conflicts(VariableDecl& decl) {
  SymbolInfo* symbol = name_lookup(current_scope_, QualifiedId{decl.name});
  if (symbol == nullptr) {
    return;
  }

  if (!symbol->is_variable()) {
    scold_user(decl, "redefinition as different kind of symbol");
  } else if (symbol->get_scope() == current_scope_) {
    scold_user(decl, "name conflicts with previous declaration");
  }
}

bool SemanticAnalyzer::visit_variable_declaration(VariableDecl& node) {
  check_for_name_conflicts(node);

  auto type = node.type->value;
  SymbolInfo& info = current_scope_->add_variable(node.name, node, type);

  if (node.initializer != nullptr) {
    if (node.initializer->type != node.type->value) {
      scold_user(node,
                 "variable must have same type as initializer: {:?} != {:?}",
                 node.type->value, node.initializer->type);
    }

    as_initializer(node.initializer);
  }

  add_to_exported_if_necessary(info);

  return true;
}
}  // namespace Front
