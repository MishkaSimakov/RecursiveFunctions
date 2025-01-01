#pragma once
#include <memory>
#include <set>

#include "Nodes.h"

class Scope {
public:
  std::unordered_map<size_t, std::unique_ptr<Type>> symbols;
  std::vector<Scope> children;
  CompoundStmt* statement;
  Scope* parent;
};

struct ASTContext {
  struct StringLiteralData {
    std::string value;
    bool is_import;

    StringLiteralData(std::string_view value, bool is_import)
        : value(value), is_import(is_import) {}

    std::string_view quoteless_view() const {
      std::string_view view = value;
      view.remove_prefix(1);
      view.remove_suffix(1);
      return view;
    }
  };

  ASTContext() = default;

  std::unique_ptr<ProgramDecl> root;
  std::vector<StringLiteralData> string_literals_table;
  std::vector<std::string> symbols;
  std::vector<size_t> imports;
  Scope root_scope;
};
