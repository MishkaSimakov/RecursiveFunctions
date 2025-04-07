#pragma once

#include "Context.h"

namespace Front {

struct FunctionSymbolInfo;

class IRFunctionDecl {
 protected:
  llvm::Function* llvm_function_;
  const FunctionSymbolInfo* info_;

 public:
  IRFunctionDecl(llvm::Function* llvm_function, const FunctionSymbolInfo* info)
      : llvm_function_(llvm_function), info_(info) {}

  llvm::Function* get_llvm_function() { return llvm_function_; }
  const FunctionSymbolInfo* get_info() { return info_; }

  bool return_through_argument() const;
};

}  // namespace Front
