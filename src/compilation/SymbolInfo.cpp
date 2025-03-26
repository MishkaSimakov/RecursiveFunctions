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
    if (!current_scope->name.has_value()) {
      throw std::runtime_error(
          "Trying to get qualified name of an entity in "
          "anonymous scope");
    }

    result.push_back(current_scope->name.value());
    current_scope = current_scope->parent;
  }

  std::ranges::reverse(result);
  return QualifiedId{std::move(result)};
}

}  // namespace Front
