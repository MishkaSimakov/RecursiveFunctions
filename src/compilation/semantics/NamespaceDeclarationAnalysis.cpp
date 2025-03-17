#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::traverse_namespace_declaration(NamespaceDecl& node) {
  NestedScopeRAII scope_guard(current_scope_);

  if (current_scope_->parent->has_symbol(node.name)) {
    auto name = context_.get_string(node.name);
    scold_user(node, fmt::format("Redefinition of namespace {}", name));
  }

  auto& info =
      current_scope_->parent->add_namespace(node.name, node, current_scope_);
  current_scope_->name = node.name;

  for (auto& decl : node.body) {
    traverse(*decl);
  }

  return true;
}
}  // namespace Front
