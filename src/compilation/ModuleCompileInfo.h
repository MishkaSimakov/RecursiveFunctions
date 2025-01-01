#pragma once

#include "ast/ASTContext.h"

struct ModuleCompileInfo {
  ASTContext context;
  std::string_view name;
  std::vector<ModuleCompileInfo*> next;
  std::vector<ModuleCompileInfo*> dependencies;
  bool is_processed;

  explicit ModuleCompileInfo(std::string_view name, ASTContext context)
      : context(std::move(context)), name(name), is_processed(false) {}
};
