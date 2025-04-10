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

  std::vector<std::string> substitutions_;

  std::string get_substitution_name(size_t index);

  void add_to_substitutions(std::string_view string);
  std::optional<std::string> consider_substitution(std::string_view string);

  std::string substitutable(std::string string);

  // types
  std::string mangle_type(Type* type);
  std::string mangle_bare_function_type(FunctionType* type);

  // names
  std::string mangle_source_name(StringId name);
  std::string mangle_source_name(std::string_view name_view);

  std::string mangle_name(const QualifiedId& name);

  std::string mangle_unscoped_name(StringId name);
  std::string mangle_nested_name(const QualifiedId& name);

 public:
  explicit Mangler(const StringPool& strings) : strings_(strings) {}

  std::string mangle(const SymbolInfo& symbol);
};

}  // namespace Front
