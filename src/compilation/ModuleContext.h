#pragma once

#include "ast/Nodes.h"
#include "compilation/Scope.h"
#include "types/TypesStorage.h"
#include "utils/StringPool.h"

namespace Front {
struct ModuleContext {
 private:
  StringPool strings_;

 public:
  enum class ModuleState {
    UNPROCESSED,
    AFTER_PARSER,
    AFTER_SEMANTIC_ANALYZER,
    AFTER_IR_COMPILER
  };
  std::string name;

  std::unique_ptr<ProgramNode> ast_root;
  std::unique_ptr<Scope> root_scope;

  // These things are generated during SemanticAnalysis
  // TODO: maybe wrap them all in another class to separate ModuleContext from
  // semantic analysis details
  TypesStorage types_storage;
  std::unordered_map<const IdExpr*, std::reference_wrapper<SymbolInfo>>
      symbols_info;
  std::unordered_map<const MemberExpr*, std::reference_wrapper<SymbolInfo>>
      members_info;
  std::unordered_map<const FunctionDecl*,
                     std::reference_wrapper<FunctionSymbolInfo>>
      functions_info;
  std::unordered_map<const StructType*, std::reference_wrapper<SymbolInfo>>
      structs_info;

  // warning: StringIds inside QualifierId are from another module.
  // To get string_view from it, get_string must be called on correct
  // module.
  std::vector<std::reference_wrapper<SymbolInfo>> exported_symbols;

  std::vector<std::reference_wrapper<ModuleContext>> dependencies;
  std::vector<std::reference_wrapper<ModuleContext>> dependents;

  ModuleState state{ModuleState::UNPROCESSED};

  ModuleContext() = default;

  StringId add_string(std::string_view string) {
    return strings_.add_string(string);
  }

  std::string_view get_string(StringId index) const {
    return strings_.get_string(index);
  }

  StringPool& get_strings_pool() { return strings_; }
  const StringPool& get_strings_pool() const { return strings_; }
};
}  // namespace Front
