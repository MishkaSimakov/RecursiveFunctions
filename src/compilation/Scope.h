#pragma once

#include "SymbolInfo.h"
#include "ast/Nodes.h"
#include "compilation/StringId.h"
#include "compilation/types/Type.h"

namespace Front {
struct Scope {
  StringId name;
  SymbolInfo* parent_symbol{nullptr};
  std::vector<std::unique_ptr<Scope>> children;
  Scope* parent{nullptr};

  std::unordered_map<StringId, SymbolInfo> symbols;

  bool has_symbol(StringId name) const { return symbols.contains(name); }

  SymbolInfo& add_namespace(StringId name, NamedDecl& decl, Scope* subscope) {
    auto info = NamespaceSymbolInfo(this, decl, subscope);
    return symbols.emplace(name, info).first->second;
  }

  SymbolInfo& add_variable(StringId name, NamedDecl& decl, Type* type) {
    auto info = VariableSymbolInfo(this, decl, type);
    return symbols.emplace(name, info).first->second;
  }

  SymbolInfo& add_function(StringId name, NamedDecl& decl, FunctionType* type) {
    auto info = FunctionSymbolInfo(this, decl, type);
    return symbols.emplace(name, info).first->second;
  }

  Scope& add_child(StringId name) {
    auto& child = children.emplace_back(std::make_unique<Scope>(name));
    child->parent = this;
    return *child;
  }

  FunctionSymbolInfo* get_parent_function() {
    Scope* current_scope = this;
    while (current_scope != nullptr) {
      if (current_scope->parent_symbol != nullptr &&
          current_scope->parent_symbol->is_function()) {
        return &std::get<FunctionSymbolInfo>(*current_scope->parent_symbol);
      }

      current_scope = current_scope->parent;
    }

    // if we are here, then we were not in a function
    return nullptr;
  }
};
}  // namespace Front
