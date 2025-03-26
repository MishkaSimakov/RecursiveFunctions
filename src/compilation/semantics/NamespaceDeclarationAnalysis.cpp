#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::traverse_namespace_declaration(NamespaceDecl& node) {
  bool prev_is_in_exported_scope = is_in_exported_scope_;

  if (current_scope_->has_symbol(node.name)) {
    SymbolInfo& info = current_scope_->symbols.at(node.name);

    if (!info.is_namespace()) {
      auto name = context_.get_string(node.name);
      scold_user(node, fmt::format("Redefinition of namespace {}", name));
    }

    current_scope_ = std::get<NamespaceSymbolInfo>(info).subscope;
  } else {
    current_scope_ = &current_scope_->add_child();

    SymbolInfo& info =
        current_scope_->parent->add_namespace(node.name, node, current_scope_);
    current_scope_->name = node.name;

    if (node.is_exported) {
      scold_user(node, "Exported namespaces are not supported yet.");
      // if (is_in_exported_scope_) {
      //   scold_user(node,
      //              "Symbol is marked \"exported\" but it is already in "
      //              "exported context. Remove unnecessary \"exported\".");
      // }
      //
      // is_in_exported_scope_ = true;
    }
  }

  for (auto& decl : node.body) {
    traverse(*decl);
  }

  current_scope_ = current_scope_->parent;
  is_in_exported_scope_ = prev_is_in_exported_scope;

  return true;
}
}  // namespace Front
