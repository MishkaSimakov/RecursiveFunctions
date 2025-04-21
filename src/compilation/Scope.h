#pragma once

#include <algorithm>

#include "SymbolInfo.h"
#include "ast/Nodes.h"
#include "compilation/types/Type.h"
#include "utils/StringId.h"

namespace Front {
struct Scope {
 private:
  StringId name_;
  std::vector<std::reference_wrapper<VariableSymbolInfo>> local_variables_;

 public:
  SymbolInfo* parent_symbol{nullptr};
  std::vector<std::unique_ptr<Scope>> children;
  Scope* parent{nullptr};

  std::unordered_map<StringId, SymbolInfo> symbols;

  explicit Scope(StringId name) : name_(name) {}

  QualifiedId get_fully_qualified_name() const {
    SymbolInfo* parent_symbol = get_parent_symbol();

    if (parent_symbol != nullptr) {
      return parent_symbol->get_fully_qualified_name();
    }

    return QualifiedId{};
  }

  StringId get_name() const { return name_; }

  void add_local_variable(VariableSymbolInfo& variable_info) {
    local_variables_.push_back(variable_info);
  }

  const auto& get_local_variables() const { return local_variables_; }

  bool has_symbol(StringId name) const { return symbols.contains(name); }

  SymbolInfo& add_struct(StringId name, Declaration& decl, Scope* subscope,
                         StructType* type) {
    auto info = StructSymbolInfo(name, this, subscope, decl, type);
    return symbols.emplace(name, info).first->second;
  }

  SymbolInfo& add_namespace(StringId name, Declaration& decl, Scope* subscope) {
    auto info = NamespaceSymbolInfo(name, this, subscope, decl);
    return symbols.emplace(name, info).first->second;
  }

  SymbolInfo& add_variable(StringId name, Declaration& decl, Type* type) {
    auto info =
        VariableSymbolInfo(name, this, decl, type, local_variables_.size());
    SymbolInfo& result = symbols.emplace(name, info).first->second;

    add_local_variable(result.as<VariableSymbolInfo>());

    return result;
  }

  SymbolInfo& add_function(StringId name, Declaration& decl, FunctionType* type,
                           Scope* subscope) {
    auto info = FunctionSymbolInfo(name, this, subscope, decl, type);
    return symbols.emplace(name, info).first->second;
  }

  SymbolInfo& add_alias(StringId name, Declaration& decl, AliasType* type) {
    auto info = TypeAliasSymbolInfo(name, this, decl, type);
    return symbols.emplace(name, info).first->second;
  }

  Scope& add_child(StringId name) {
    auto& child = children.emplace_back(std::make_unique<Scope>(name));
    child->parent = this;
    return *child;
  }

  SymbolInfo* get_parent_symbol() const {
    const Scope* scope = this;
    while (scope != nullptr) {
      if (scope->parent_symbol != nullptr) {
        return scope->parent_symbol;
      }

      scope = scope->parent;
    }

    return nullptr;
  }
};
}  // namespace Front
