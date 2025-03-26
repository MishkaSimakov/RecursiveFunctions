#pragma once

#include "ast/Nodes.h"
#include "types/Type.h"

namespace Front {
struct Scope;

// Symbols Table Entry
// it can represent:
// 1. variable name
// 2. named type (class, typedef etc.)
// 3. function name
// 4. namespace

struct SymbolInfo;

struct InternalSymbolInfo {
  Scope* scope;
  NamedDecl& declaration;

  StringId get_unqualified_name() const { return declaration.name; }
  QualifiedId get_fully_qualified_name() const;
};

struct VariableSymbolInfo : InternalSymbolInfo {
  Type* type;

  VariableSymbolInfo(Scope* scope, NamedDecl& declaration, Type* type)
      : InternalSymbolInfo(scope, declaration), type(type) {}
};

struct FunctionSymbolInfo : InternalSymbolInfo {
  FunctionType* type;
  std::vector<std::reference_wrapper<VariableSymbolInfo>> local_variables;

  FunctionSymbolInfo(Scope* scope, NamedDecl& declaration, FunctionType* type)
      : InternalSymbolInfo(scope, declaration), type(type) {}
};

struct NamespaceSymbolInfo : InternalSymbolInfo {
  Scope* subscope;

  NamespaceSymbolInfo(Scope* scope, NamedDecl& declaration, Scope* subscope)
      : InternalSymbolInfo(scope, declaration), subscope(subscope) {}
};

using SymbolInfoVariant =
    std::variant<VariableSymbolInfo, NamespaceSymbolInfo, FunctionSymbolInfo>;

struct SymbolInfo : SymbolInfoVariant {
  using SymbolInfoVariant::SymbolInfoVariant;
  using SymbolInfoVariant::operator=;

  StringId get_unqualified_name() const {
    return std::visit(
        [](const auto& value) { return value.get_unqualified_name(); }, *this);
  }

  QualifiedId get_fully_qualified_name() const {
    return std::visit(
        [](const auto& value) { return value.get_fully_qualified_name(); },
        *this);
  }

  bool is_variable() const {
    return std::holds_alternative<VariableSymbolInfo>(*this);
  }

  bool is_function() const {
    return std::holds_alternative<FunctionSymbolInfo>(*this);
  }

  bool is_namespace() const {
    return std::holds_alternative<NamespaceSymbolInfo>(*this);
  }
};

}  // namespace Front
