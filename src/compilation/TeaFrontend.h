#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "GlobalContext.h"
#include "compilation/ModuleContext.h"
#include "compilation/types/TypesStorage.h"
#include "sources/SourceManager.h"
#include "utils/OneShotObject.h"

namespace Front {
struct ModuleCompileInfo {
  bool is_processed = false;
  std::vector<ModuleCompileInfo*> next;
  std::vector<ModuleCompileInfo*> dependencies;
  ModuleContext& context;

  ModuleCompileInfo(ModuleContext& context) : context(context) {}
};

class TeaFrontend : OneShotObject {
  std::unique_ptr<llvm::LLVMContext> llvm_context_;
  std::vector<std::unique_ptr<llvm::Module>> llvm_modules_;

  const std::unordered_map<std::string, std::filesystem::path>& files_;
  std::filesystem::path output_file_;

  GlobalContext context_;

  std::vector<std::string_view> find_loops() const;

  void build_ast();
  void build_symbols_table_and_compile();

 public:
  explicit TeaFrontend(
      const std::unordered_map<std::string, std::filesystem::path>& files,
      std::filesystem::path output_file)
      : llvm_context_(std::make_unique<llvm::LLVMContext>()),
        files_(files),
        output_file_(std::move(output_file)) {}

  int compile();
};
}  // namespace Front
