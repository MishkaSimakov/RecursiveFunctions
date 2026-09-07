#include "SemanticAnalyzer.h"

namespace Front {

bool SemanticAnalyzer::visit_type_alias_declaration(TypeAliasDecl& node) {
  // alias on alias is reduced:
  // A: type == B; C: type == B;
  // in this case C.original = A
  Type* original = node.original->value->get_original();

  // aliases are strong in TeaLang
  // therefore separate type is created for alias
  auto name =
      QualifiedId::merge(current_scope_->get_fully_qualified_name(), node.name);

  AliasType* type = types().add_alias(name, original);
  SymbolInfo& info = current_scope_->add_alias(node.name, node, type);

  add_to_exported_if_necessary(info);

  return true;
}

}  // namespace Front
