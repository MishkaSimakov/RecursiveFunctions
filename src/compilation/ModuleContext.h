#pragma once

#include <set>

#include "ast/Nodes.h"
#include "compilation/Scope.h"
#include "compilation/StringId.h"
#include "types/TypesStorage.h"

namespace Front {
struct ModuleContext {
  enum class ModuleState {
    UNPROCESSED,
    AFTER_PARSER,
    AFTER_SEMANTIC_ANALYZER,
    AFTER_IR_COMPILER
  };
  std::string name;

  TypesStorage types_storage;
  std::unique_ptr<ProgramDecl> ast_root;
  std::unique_ptr<Scope> root_scope;

  std::unordered_map<const IdExpr*, std::reference_wrapper<SymbolInfo>>
      symbols_info;
  std::unordered_map<const FunctionDecl*, std::reference_wrapper<SymbolInfo>>
      functions_info;

  // warning: StringIds inside QualifierId are from another module.
  // To get string_view from it, get_string must be called on correct module.
  std::unordered_map<QualifiedId, std::reference_wrapper<SymbolInfo>>
      exported_symbols;

  // I use std::less<void> to compare std::string with std::string_view without
  // creating new string from string_view
  std::set<std::string, std::less<>> strings_table;

  std::vector<std::reference_wrapper<ModuleContext>> dependencies;
  std::vector<std::reference_wrapper<ModuleContext>> dependents;

  ModuleState state{ModuleState::UNPROCESSED};

  ModuleContext() = default;

  StringId add_string(std::string_view string) {
    auto [itr, _] = strings_table.emplace(string);
    return StringId(itr);
  }

  std::string_view get_string(StringId index) const { return *index.itr_; }
};
}  // namespace Front
