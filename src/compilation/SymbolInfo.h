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

struct BaseSymbolInfo {
  Scope* scope;
  Declaration& declaration;

  StringId get_unqualified_name() const { return declaration.name; }
  QualifiedId get_fully_qualified_name() const;
  DeclarationSpecifiers get_specifiers() const {
    return declaration.specifiers;
  }
};

struct VariableSymbolInfo;

// class or function
struct ScopefulSymbolInfo : BaseSymbolInfo {
  Scope* subscope;

  ScopefulSymbolInfo(Scope* scope, Declaration& declaration, Scope* subscope)
      : BaseSymbolInfo(scope, declaration), subscope(subscope) {}
};

struct VariableSymbolInfo : BaseSymbolInfo {
  Type* type;

  VariableSymbolInfo(Scope* scope, Declaration& declaration, Type* type)
      : BaseSymbolInfo(scope, declaration), type(type) {}

  VariableDecl& get_decl() const {
    return static_cast<VariableDecl&>(declaration);
  }
};

struct FunctionSymbolInfo : ScopefulSymbolInfo {
  FunctionType* type;
  std::vector<std::reference_wrapper<VariableSymbolInfo>> local_variables;

  FunctionSymbolInfo(Scope* scope, Scope* subscope, Declaration& declaration,
                     FunctionType* type)
      : ScopefulSymbolInfo(scope, declaration, subscope), type(type) {}

  FunctionDecl& get_decl() const {
    return static_cast<FunctionDecl&>(declaration);
  }
};

struct NamespaceSymbolInfo : ScopefulSymbolInfo {
  NamespaceSymbolInfo(Scope* scope, Scope* subscope, Declaration& declaration)
      : ScopefulSymbolInfo(scope, declaration, subscope) {}
};

struct ClassSymbolInfo : ScopefulSymbolInfo {
  ClassType* type{nullptr};

  ClassSymbolInfo(Scope* scope, Scope* subscope, Declaration& declaration)
      : ScopefulSymbolInfo(scope, declaration, subscope) {}
};

struct TypeAliasSymbolInfo : BaseSymbolInfo {
  AliasType* type;

  TypeAliasSymbolInfo(Scope* scope, Declaration& declaration, AliasType* type)
      : BaseSymbolInfo(scope, declaration), type(type) {}
};

using SymbolInfoVariant =
    std::variant<VariableSymbolInfo, NamespaceSymbolInfo, FunctionSymbolInfo,
                 ClassSymbolInfo, TypeAliasSymbolInfo>;

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

  Declaration& get_declaration() const {
    return std::visit(
        [](const auto& value) -> Declaration& { return value.declaration; },
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

  Type* get_type() const;
};

}  // namespace Front
