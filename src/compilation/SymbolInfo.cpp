#include "SymbolInfo.h"

#include <algorithm>

#include "Scope.h"
#include "utils/TupleUtils.h"

namespace Front {

QualifiedId InternalSymbolInfo::get_fully_qualified_name() const {
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

}  // namespace Front
