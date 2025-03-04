#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::before_namespace_declaration(NamespaceDecl& node) {
  auto namespace_parent_scope = current_scope_;
  auto namespace_subscope = start_nested_scope();

  auto [_, was_emplaced] = namespace_parent_scope->symbols.emplace(
      node.name, SymbolInfo::make_namespace(node, namespace_subscope));

  if (!was_emplaced) {
    auto name = context_.get_string(node.name);
    scold_user(node, fmt::format("Redefinition of namespace {}", name));
  }

  return true;
}

bool SemanticAnalyzer::after_namespace_declaration(NamespaceDecl& node) {
  end_nested_scope();
  return true;
}
}  // namespace Front
