#pragma once

#include <string>

#include "ast/Nodes.h"
#include "compilation/Scope.h"
#include "utils/StringPool.h"

namespace Front {

struct MangledPart {
  std::string value;
  bool is_substituent;
};

class MangledMess {
 private:
  std::vector<MangledPart> parts_;
  std::vector<std::string> substitutions_;

 public:
  MangledMess() = default;

  MangledMess(std::string value, bool is_substituent) {
    append(value, is_substituent);
  }

  void append(MangledMess other) {
    if (other.empty()) {
      return;
    }

    parts_.append_range(other.parts_);
    substitutions_.append_range(other.substitutions_);

    substitutions_.push_back(join_all());
  }

  void append(std::string value, bool is_substituent) {
    parts_.emplace_back(value, is_substituent);

    if (is_substituent) {
      substitutions_.push_back(join_all());
    }
  }

  std::string join_all() const {
    std::string result;
    for (auto& part : parts_) {
      result += part.value;
    }
    return result;
  }

  bool empty() const { return parts_.empty(); }
};

/* This is a feeble attempt to create a mangler. Itanium C++ ABI is used:
 * https://itanium-cxx-abi.github.io/cxx-abi/
 *
 * Mangling in the ABI is represented by grammar. Names of methods of this class
 * try to be of the form mangle_*, where * is a name of grammar non-terminal
 */
class Mangler {
  const StringPool& strings_;

  // types
  MangledMess mangle_type(Type* type) const;
  MangledMess mangle_bare_function_type(FunctionType* type) const;

  MangledMess mangle_source_name(StringId name) const;
  MangledMess mangle_source_name(std::string_view name_view) const;

  MangledMess mangle_name(const QualifiedId& name) const;

 public:
  explicit Mangler(const StringPool& strings) : strings_(strings) {}

  std::string mangle(const SymbolInfo& symbol) const;
};

}  // namespace Front
