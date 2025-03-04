#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "GlobalContext.h"
#include "compilation/ModuleContext.h"
#include "compilation/types/TypesStorage.h"
#include "sources/SourceManager.h"

namespace Front {
struct ModuleCompileInfo {
  bool is_processed = false;
  std::vector<ModuleCompileInfo*> next;
  std::vector<ModuleCompileInfo*> dependencies;
  ModuleContext& context;

  ModuleCompileInfo(ModuleContext& context) : context(context) {}
};

class TeaFrontend {
  const std::unordered_map<std::string, std::filesystem::path>& files_;
  GlobalContext context_;

  std::vector<std::string_view> find_loops() const;

  void build_ast();
  void build_symbols_table();

 public:
  explicit TeaFrontend(
      const std::unordered_map<std::string, std::filesystem::path>& files)
      : files_(files) {}

  int compile();
};
}  // namespace Front
