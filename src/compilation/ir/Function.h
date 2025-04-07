#pragma once

#include "FunctionDecl.h"

namespace Front {
class IRFunction : public IRFunctionDecl {
 private:
  llvm::BasicBlock* alloca_block_{nullptr};
  llvm::BasicBlock* return_block_{nullptr};
  llvm::Value* return_value_{nullptr};

 public:
  IRFunction(IRFunctionDecl declaration, llvm::BasicBlock* alloca_block,
             llvm::BasicBlock* return_block, llvm::Value* return_value)
      : IRFunctionDecl(declaration),
        alloca_block_(alloca_block),
        return_block_(return_block),
        return_value_(return_value) {}

  llvm::BasicBlock* get_alloca_block() { return alloca_block_; }
  llvm::BasicBlock* get_return_block() { return return_block_; }

  llvm::Value* get_return_value() { return return_value_; }
};

}  // namespace Front
