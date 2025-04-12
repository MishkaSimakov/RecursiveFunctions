#pragma once

#include "SymbolInfo.h"
#include "ast/Nodes.h"
#include "compilation/types/Type.h"
#include "utils/StringId.h"

namespace Front {
struct Scope {
  StringId name;
  SymbolInfo* parent_symbol{nullptr};
  std::vector<std::unique_ptr<Scope>> children;
  Scope* parent{nullptr};

  std::unordered_map<StringId, SymbolInfo> symbols;

  bool has_symbol(StringId name) const { return symbols.contains(name); }

  SymbolInfo& add_namespace(StringId name, Declaration& decl, Scope* subscope) {
    auto info = NamespaceSymbolInfo(this, subscope, decl);
    return symbols.emplace(name, info).first->second;
  }

  SymbolInfo& add_variable(StringId name, Declaration& decl, Type* type) {
    auto info = VariableSymbolInfo(this, decl, type);
    return symbols.emplace(name, info).first->second;
  }

  SymbolInfo& add_function(StringId name, Declaration& decl, FunctionType* type,
                           Scope* subscope) {
    auto info = FunctionSymbolInfo(this, subscope, decl, type);
    return symbols.emplace(name, info).first->second;
  }

  Scope& add_child(StringId name) {
    auto& child = children.emplace_back(std::make_unique<Scope>(name));
    child->parent = this;
    return *child;
  }

  SymbolInfo* get_parent_symbol() {
    Scope* scope = this;
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
