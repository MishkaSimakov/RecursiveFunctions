#pragma once

#include <string>

#include "ast/Nodes.h"
#include "compilation/Scope.h"
#include "utils/StringPool.h"

namespace Front {

/* This is a feeble attempt to create a mangler. Itanium C++ ABI is used:
 * https://itanium-cxx-abi.github.io/cxx-abi/
 *
 * Mangling in the ABI is represented by grammar. Names of methods of this class
 * try to be of the form mangle_*, where * is a name of grammar non-terminal
 */
class Mangler {
  const StringPool& strings_;

  // types
  std::string mangle_type(Type* type) const;
  std::string mangle_bare_function_type(FunctionType* type) const;

  std::string mangle_source_name(StringId name) const;
  std::string mangle_source_name(std::string_view name_view) const;

  std::string mangle_name(const QualifiedId& name) const;

 public:
  explicit Mangler(const StringPool& strings) : strings_(strings) {}

  std::string mangle(const SymbolInfo& symbol) const;
};

}  // namespace Front
