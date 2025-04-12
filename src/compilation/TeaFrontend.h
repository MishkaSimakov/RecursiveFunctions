#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "FrontendConfiguration.h"
#include "GlobalContext.h"
#include "utils/OneShotObject.h"

namespace Front {

class TeaFrontend : OneShotObject {
  std::unique_ptr<llvm::LLVMContext> llvm_context_;
  std::vector<std::unique_ptr<llvm::Module>> llvm_modules_;

  std::unordered_map<std::string, std::filesystem::path> files_;
  std::filesystem::path output_file_;
  EmitType emit_type_;

  GlobalContext context_;

  std::vector<std::string_view> find_loops() const;

  void build_ast();
  void build_symbols_table_and_compile();

  void emit_ast() const;
  void emit_ir(const llvm::Module& main_module) const;

 public:
  explicit TeaFrontend(TeaFrontendConfiguration config);

  int compile();
};
}  // namespace Front
