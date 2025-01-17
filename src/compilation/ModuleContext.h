#pragma once

#include <deque>

#include "ast/Nodes.h"
#include "compilation/Scope.h"
#include "compilation/StringId.h"

struct ModuleContext {
  size_t id;

  std::unique_ptr<ProgramDecl> ast_root;
  std::vector<StringId> imports;

  Scope* root_scope{nullptr};
  std::deque<Scope> scopes;

  bool is_preprocessed;

  explicit ModuleContext(size_t id) : id(id), is_preprocessed(false) {}
};
