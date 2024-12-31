#pragma once
#include <memory>

#include "Nodes.h"

struct ASTContext {
  struct StringLiteralData {
    std::string value;
    bool is_import;

    StringLiteralData(std::string_view value, bool is_import)
        : value(value), is_import(is_import) {}
  };

  std::unique_ptr<ProgramDecl> root;
  std::vector<StringLiteralData> string_literals_table;
  std::vector<size_t> dependencies;
};
