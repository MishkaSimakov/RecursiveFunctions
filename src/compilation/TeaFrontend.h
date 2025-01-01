#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "ast/ASTContext.h"
#include "compilation/ModuleCompileInfo.h"
#include "sources/SourceManager.h"
#include "compilation/types/TypesStorage.h"

class TeaFrontend {
  std::unordered_map<std::string_view, ModuleCompileInfo> modules_;
  std::vector<ModuleCompileInfo*> start_modules_;
  TypesStorage types_storage_;
  SourceManager source_manager_;

  static bool has_loops_recursive(
      const ModuleCompileInfo* current,
      std::unordered_map<const ModuleCompileInfo*, int>& colors);
  bool has_loops() const;

  void build_ast_and_dependencies(
      const std::unordered_map<std::string, std::filesystem::path>& files);

  bool try_compile_module(ModuleCompileInfo* job);

 public:
  int compile(
      const std::unordered_map<std::string, std::filesystem::path>& files);
};
