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

  explicit Scope(Scope* parent) : parent(parent) {}

  bool has_symbol(StringId name) const { return symbols.contains(name); }

  SymbolInfo& add_namespace(StringId name, NamespaceDecl& decl,
                            Scope* subscope) {
    return symbols
        .emplace(name, SymbolInfo{this, decl, NamespaceSymbol{subscope}})
        .first->second;
  }

  SymbolInfo& add_variable(StringId name, Declaration& decl, Type* type) {
    return symbols.emplace(name, SymbolInfo{this, decl, VariableSymbol{type}})
        .first->second;
  }

  SymbolInfo& add_function(StringId name, Declaration& decl,
                           FunctionType* type) {
    return symbols.emplace(name, SymbolInfo{this, decl, FunctionSymbol{type}})
        .first->second;
  }

  // returns nullptr for anonymous scopes
  SymbolInfo* get_scope_info() const {
    if (!name.has_value()) {
      return nullptr;
    }

    return &parent->symbols.at(name.value());
  }
};
}  // namespace Front
