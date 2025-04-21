#include "SymbolInfo.h"

#include <algorithm>

#include "Scope.h"
#include "utils/TupleUtils.h"

namespace Front {

QualifiedId BaseSymbolInfo::get_fully_qualified_name() const {
  std::vector<StringId> result;
  result.push_back(declaration.name);

  Scope* current_scope = scope;
  while (current_scope->parent != nullptr) {
    result.push_back(current_scope->name);
    current_scope = current_scope->parent;
  }

  std::ranges::reverse(result);
  return QualifiedId{std::move(result)};
}

Type* SymbolInfo::get_type() const {
  return std::visit(
      Overloaded{
          [](const StructSymbolInfo& info) -> Type* { return info.type; },
          [](const TypeAliasSymbolInfo& info) -> Type* { return info.type; },
          [](const auto&) -> Type* {
            unreachable("type must not be accessed for other symbol types.");
          }},
      *this);
}

bool SymbolInfo::is_inside(Scope* scope) const {
  Scope* current_scope = get_scope();

  while (current_scope != nullptr) {
    if (current_scope == scope) {
      return true;
    }

    current_scope = current_scope->parent;
  }

  return false;
}

}  // namespace Front
