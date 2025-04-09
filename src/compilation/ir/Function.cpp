#include "Function.h"

#include <llvm/IR/Function.h>

namespace Front {

llvm::Value* IRFunction::get_llvm_argument(size_t index) {
  if (return_through_argument()) {
    ++index;
  }

  return llvm_function_->getArg(index);
}

}  // namespace Front
