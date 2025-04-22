#include "Constants.h"
#include "SemanticAnalyzer.h"
#include "ast/ASTConstructor.h"

namespace Front {

bool SemanticAnalyzer::traverse_class_declaration(ClassDecl& node) {
  Scope* subscope = &current_scope_->add_child(node.name);

  {
    NestedScopeRAII scope_guard(*this, *subscope);

    for (auto& decl : node.body) {
      traverse(*decl);
    }
  }

  std::vector<std::pair<StringId, Type*>> members;
  for (VariableSymbolInfo& local_variable : subscope->get_local_variables()) {
    members.emplace_back(local_variable.declaration.name, local_variable.type);
  }

  auto name =
      QualifiedId::merge(current_scope_->get_fully_qualified_name(), node.name);

  StructType* type =
      types().make_type<StructType>(std::move(name), std::move(members));
  SymbolInfo& info =
      current_scope_->add_struct(node.name, node, subscope, type);

  add_to_exported_if_necessary(info);

  context_.structs_info.emplace(type, info);

  return true;
}

}  // namespace Front
