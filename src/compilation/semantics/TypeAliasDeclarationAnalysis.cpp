#include "SemanticAnalyzer.h"

namespace Front {

bool SemanticAnalyzer::visit_type_alias_declaration(TypeAliasDecl& node) {
  // alias on alias is reduced:
  // A: type == B; C: type == B;
  // in this case C.original = A
  Type* original = node.original->value;
  if (original->get_kind() == Type::Kind::ALIAS) {
    original = static_cast<AliasType*>(original)->original;
  }

  // aliases are strong in TeaLang
  // therefore separate type is created for alias
  auto [itr, was_emplaced] = current_scope_->symbols.emplace(
      node.name, TypeAliasSymbolInfo(current_scope_, node, nullptr));
  SymbolInfo& info = itr->second;
  TypeAliasSymbolInfo& alias_info = std::get<TypeAliasSymbolInfo>(info);

  auto qualified_name = info.get_fully_qualified_name();
  alias_info.type = types().add_alias(std::move(qualified_name), original);

  add_to_exported_if_necessary(info);

  return true;
}

}  // namespace Front
