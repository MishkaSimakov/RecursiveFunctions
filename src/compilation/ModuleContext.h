#pragma once

#include <set>

#include "ast/Nodes.h"
#include "compilation/Scope.h"
#include "compilation/StringId.h"
#include "types/TypesStorage.h"

namespace Front {
struct ModuleContext {
  std::string name;

  TypesStorage types_storage;
  std::unique_ptr<ProgramDecl> ast_root;
  std::unique_ptr<Scope> root_scope;

  // I use std::less<void> to compare std::string with std::string_view without
  // creating new string from string_view
  std::set<std::string, std::less<>> strings_table;

  std::vector<std::reference_wrapper<ModuleContext>> dependencies;
  std::vector<std::reference_wrapper<ModuleContext>> dependents;

  bool has_symbols_table{false};

  ModuleContext() = default;

  StringId add_string(std::string_view string) {
    auto [itr, _] = strings_table.emplace(string);
    return StringId(itr);
  }

  std::string_view get_string(StringId index) const { return *index.itr_; }
};
}  // namespace Front
