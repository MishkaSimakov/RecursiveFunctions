#pragma once

#include "ast/ASTVisitor.h"

class ImportASTVisitor : public ASTVisitor<ImportASTVisitor, false> {
 public:
  using ASTVisitor::ASTVisitor;

  bool visit_import_declaration(ImportDecl& import_decl) {
    context_.imports.push_back(import_decl.id);
    return true;
  }

  // any other declaration must stop the search
  bool visit_function_declaration(FunctionDecl&) { return false; }
};
