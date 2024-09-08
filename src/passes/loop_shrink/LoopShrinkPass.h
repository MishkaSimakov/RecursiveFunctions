#pragma once
#include "passes/pass_types/FunctionLevelPass.h"

namespace Passes {
class LoopShrinkPass : public FunctionLevelPass<> {
 public:
  LoopShrinkPass(PassManager& manager) : FunctionLevelPass(manager) {
    info_.name = "Loop shrink";
    info_.repeat_while_changing = false;

    info_.preserve_ssa = true;
    info_.require_ssa = true;
  }

 protected:
  bool apply(IR::Function& function) override;
};
}  // namespace Passes
