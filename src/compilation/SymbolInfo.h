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
  Scope* scope;
  Declaration& declaration;
  std::variant<VariableSymbol, FunctionSymbol, NamespaceSymbol> data;

  SymbolInfo(Scope* scope, Declaration& declaration,
             std::variant<VariableSymbol, FunctionSymbol, NamespaceSymbol> data)
      : scope(scope), declaration(declaration), data(data) {}

  bool is_variable() const {
    return std::holds_alternative<VariableSymbol>(data);
  }

  bool is_function() const {
    return std::holds_alternative<FunctionSymbol>(data);
  }

  bool is_namespace() const {
    return std::holds_alternative<NamespaceSymbol>(data);
  }

  StringId get_unqualified_name() const {
    return static_cast<const NamedDecl&>(declaration).name;
  }

  QualifiedId get_fully_qualified_name() const;
};
}  // namespace Front
