#include "Context.h"

#include <llvm/IR/Module.h>

llvm::LLVMContext& Front::IRContext::get_llvm_context() {
  return llvm_module.getContext();
}
