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
  StringId name;
  Scope* scope;
  Declaration& declaration;

  StringId get_unqualified_name() const { return name; }
  QualifiedId get_fully_qualified_name() const;
  DeclarationSpecifiers get_specifiers() const {
    return declaration.specifiers;
  }
};

struct VariableSymbolInfo;

// class or function
struct ScopefulSymbolInfo : BaseSymbolInfo {
  Scope* subscope;

  ScopefulSymbolInfo(StringId name, Scope* scope, Declaration& declaration, Scope* subscope)
      : BaseSymbolInfo(name, scope, declaration), subscope(subscope) {}
};

struct VariableSymbolInfo : BaseSymbolInfo {
  Type* type;
  size_t index;

  VariableSymbolInfo(StringId name, Scope* scope, Declaration& declaration, Type* type, size_t index)
      : BaseSymbolInfo(name, scope, declaration), type(type), index(index) {}

  VariableDecl& get_decl() const {
    return static_cast<VariableDecl&>(declaration);
  }
};

struct FunctionSymbolInfo : ScopefulSymbolInfo {
  FunctionType* type;
  Type* transformation_type{nullptr};

  FunctionSymbolInfo(StringId name, Scope* scope, Scope* subscope, Declaration& declaration,
                     FunctionType* type)
      : ScopefulSymbolInfo(name, scope, declaration, subscope), type(type) {}

  FunctionDecl& get_decl() const {
    return static_cast<FunctionDecl&>(declaration);
  }
};

struct NamespaceSymbolInfo : ScopefulSymbolInfo {
  NamespaceSymbolInfo(StringId name, Scope* scope, Scope* subscope, Declaration& declaration)
      : ScopefulSymbolInfo(name, scope, declaration, subscope) {}
};

struct StructSymbolInfo : ScopefulSymbolInfo {
  StructType* type;

  StructSymbolInfo(StringId name, Scope* scope, Scope* subscope, Declaration& declaration,
                   StructType* type)
      : ScopefulSymbolInfo(name, scope, declaration, subscope), type(type) {}
};

struct TypeAliasSymbolInfo : BaseSymbolInfo {
  AliasType* type;

  TypeAliasSymbolInfo(StringId name, Scope* scope, Declaration& declaration, AliasType* type)
      : BaseSymbolInfo(name, scope, declaration), type(type) {}
};

using SymbolInfoVariant =
    std::variant<VariableSymbolInfo, NamespaceSymbolInfo, FunctionSymbolInfo,
                 StructSymbolInfo, TypeAliasSymbolInfo>;

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

  Scope* get_scope() const {
    return std::visit([](const auto& value) -> Scope* { return value.scope; },
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

  bool is_class() const {
    return std::holds_alternative<StructSymbolInfo>(*this);
  }

  Type* get_type() const;

  bool is_inside(Scope* scope) const;

  template <typename T>
    requires std::is_base_of_v<BaseSymbolInfo, T>
  T& as() {
    if constexpr (Constants::debug) {
      return std::get<T>(*this);
    } else {
      // this can be optimized to basically no-op,
      // therefore use this access option in production
      return *std::get_if<T>(this);
    }
  }

  template <typename T>
    requires std::is_base_of_v<BaseSymbolInfo, T>
  const T& as() const {
    if constexpr (Constants::debug) {
      return std::get<T>(*this);
    } else {
      // this can be optimized to basically no-op,
      // therefore use this access option in production
      return *std::get_if<T>(this);
    }
  }
};

}  // namespace Front
