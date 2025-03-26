#pragma once

#include "SymbolInfo.h"
#include "ast/Nodes.h"
#include "compilation/StringId.h"
#include "compilation/types/Type.h"

namespace Front {
struct Scope {
  std::optional<StringId> name;
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

  // returns nullptr for anonymous scopes
  SymbolInfo* get_scope_info() const {
    if (!name.has_value()) {
      return nullptr;
    }

    return &parent->symbols.at(name.value());
  }

  Scope& add_child() {
    auto& child = children.emplace_back(std::make_unique<Scope>());
    child->parent = this;
    return *child;
  }
};
}  // namespace Front
