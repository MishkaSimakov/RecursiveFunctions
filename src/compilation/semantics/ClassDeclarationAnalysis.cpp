#include "SemanticAnalyzer.h"

namespace Front {

bool SemanticAnalyzer::traverse_class_declaration(ClassDecl& node) {
  Scope* subscope = &current_scope_->add_child(node.name);
  auto info = ClassSymbolInfo(current_scope_, subscope, node);

  auto qualified_name = info.get_fully_qualified_name();
  info.type = types().make_type<ClassType>(std::move(qualified_name));

  auto [itr, was_emplaced] = current_scope_->symbols.emplace(node.name, info);

  subscope->parent_symbol = &itr->second;
  if (node.specifiers.is_exported()) {
    context_.exported_symbols.push_back(itr->second);
  }

  NestedScopeRAII scope_guard(*this, *subscope);

  for (auto& decl : node.body) {
    traverse(*decl);
  }

  return true;
}

}  // namespace Front
