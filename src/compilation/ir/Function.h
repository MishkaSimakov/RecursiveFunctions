#pragma once

#include "Context.h"

namespace Front {
struct FunctionSymbolInfo;

class IRFunction {
 private:
  llvm::Function* llvm_function_;
  FunctionSymbolInfo* info_;

  llvm::BasicBlock* alloca_block_{nullptr};
  llvm::BasicBlock* return_block_{nullptr};
  llvm::Value* return_value_{nullptr};

 public:
  IRFunction(llvm::Function* llvm_function, FunctionSymbolInfo* info,
             llvm::BasicBlock* alloca_block, llvm::BasicBlock* return_block,
             llvm::Value* return_value)
      : llvm_function_(llvm_function),
        info_(info),
        alloca_block_(alloca_block),
        return_block_(return_block),
        return_value_(return_value) {}

  llvm::Function* get_llvm_function() { return llvm_function_; }
  FunctionSymbolInfo* get_info() { return info_; }

  llvm::BasicBlock* get_alloca_block() { return alloca_block_; }
  llvm::BasicBlock* get_return_block() { return return_block_; }

  llvm::Value* get_return_value() { return return_value_; }
};

}  // namespace Front
