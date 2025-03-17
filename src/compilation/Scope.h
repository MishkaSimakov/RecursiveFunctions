#pragma once

#include "ast/Nodes.h"
#include "compilation/StringId.h"
#include "compilation/types/Type.h"

namespace Front {
// Symbols Table Entry
// it can represent:
// 1. variable name
// 2. named type (class, typedef etc.)
// 3. function name
// 4. namespace
struct Scope;

// Variable or Parameter, these symbols are in the end of qualified id
struct VariableSymbol {
  Type* type;
};

struct FunctionSymbol {
  FunctionType* type;
};

struct NamespaceSymbol {
  Scope* subscope;
};

struct SymbolInfo {
 private:
  using NodeKind = ASTNode::Kind;

 public:
  Declaration& declaration;
  std::variant<VariableSymbol, FunctionSymbol, NamespaceSymbol> data;

  SymbolInfo(Declaration& declaration,
             std::variant<VariableSymbol, FunctionSymbol, NamespaceSymbol> data)
      : declaration(declaration), data(data) {}

  bool is_variable() const {
    return std::holds_alternative<VariableSymbol>(data);
  }

  bool is_function() const {
    return std::holds_alternative<FunctionSymbol>(data);
  }
};

struct Scope {
  std::optional<StringId> name;
  std::vector<std::unique_ptr<Scope>> children;
  Scope* parent{nullptr};

  std::unordered_map<StringId, SymbolInfo> symbols;

  explicit Scope(Scope* parent) : parent(parent) {}

  bool has_symbol(StringId name) const { return symbols.contains(name); }

  SymbolInfo& add_namespace(StringId name, NamespaceDecl& decl,
                            Scope* subscope) {
    return symbols.emplace(name, SymbolInfo{decl, NamespaceSymbol{subscope}})
        .first->second;
  }

  SymbolInfo& add_variable(StringId name, Declaration& decl, Type* type) {
    return symbols.emplace(name, SymbolInfo{decl, VariableSymbol{type}})
        .first->second;
  }

  SymbolInfo& add_function(StringId name, Declaration& decl,
                           FunctionType* type) {
    return symbols.emplace(name, SymbolInfo{decl, FunctionSymbol{type}})
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
