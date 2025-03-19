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

// Variable, Function or Parameter
// this symbols are in the end of qualified id
struct TerminalSymbol {
  Type* type;
};

struct NamespaceSymbol {
  Scope* subscope;
};

struct SymbolInfo {
 private:
  using NodeKind = ASTNode::Kind;

 public:
  Declaration& declaration;
  std::variant<TerminalSymbol, NamespaceSymbol> data;

  static SymbolInfo make_namespace(NamespaceDecl& decl, Scope* subscope) {
    return SymbolInfo{decl, NamespaceSymbol{subscope}};
  }

  static SymbolInfo make_terminal(Declaration& decl, Type* type) {
    return SymbolInfo{decl, TerminalSymbol{type}};
  }
};

struct Scope {
  std::vector<std::unique_ptr<Scope>> children;
  Scope* parent{nullptr};

  std::unordered_map<StringId, SymbolInfo> symbols;

  explicit Scope(Scope* parent) : parent(parent) {}
};
}  // namespace Front
