#pragma once

#include "Context.h"

namespace Front {

struct FunctionSymbolInfo;

class IRFunctionDecl {
 protected:
  llvm::Function* llvm_function_;
  FunctionSymbolInfo* info_;

 public:
  IRFunctionDecl(llvm::Function* llvm_function, FunctionSymbolInfo* info)
      : llvm_function_(llvm_function), info_(info) {}

 bool return_through_argument() const;
};

}  // namespace Front
