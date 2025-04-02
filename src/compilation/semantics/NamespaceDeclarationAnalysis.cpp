#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::traverse_namespace_declaration(NamespaceDecl& node) {
  Scope* subscope;

  if (current_scope_->has_symbol(node.name)) {
    SymbolInfo& info = current_scope_->symbols.at(node.name);

    if (!info.is_namespace()) {
      auto name = context_.get_string(node.name);
      scold_user(node,
                 fmt::format("redefinition of {:?} as different kind of symbol",
                             name));
    }

    subscope = std::get<NamespaceSymbolInfo>(info).subscope;
  } else {
    subscope = &current_scope_->add_child(node.name);
    SymbolInfo& info = current_scope_->add_namespace(node.name, node, subscope);

    add_to_exported_if_necessary(info);
  }

  NestedScopeRAII scope_guard(*this, *subscope);

  for (auto& decl : node.body) {
    traverse(*decl);
  }

  return true;
}
}  // namespace Front
