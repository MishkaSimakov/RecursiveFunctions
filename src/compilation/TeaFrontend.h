#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "GlobalContext.h"
#include "compilation/ModuleContext.h"
#include "compilation/types/TypesStorage.h"
#include "sources/SourceManager.h"

struct ModuleCompileInfo {
  bool is_processed = false;
  std::vector<ModuleCompileInfo*> next;
  std::vector<ModuleCompileInfo*> dependencies;
  ModuleContext& context;

  ModuleCompileInfo(ModuleContext& context) : context(context) {}
};

class TeaFrontend {
  GlobalContext context_;

  std::vector<ModuleCompileInfo*> start_modules_;
  std::unordered_map<size_t, ModuleCompileInfo> compile_info_;

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
