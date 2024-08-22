#pragma once
#include "passes/pass_types/BasicBlockLevelPass.h"

// After register allocation pass many virtual registers are mapped into one
// register. Sometimes instructions like
// "mov x0, x0" or "mov x0, x1; mov x1, x0" are generated afterwards.
// Aim of this pass is to remove instructions like this.

namespace Passes {
class SillyMoveErasurePass : public BasicBlockLevelPass<> {
 public:
  SillyMoveErasurePass(PassManager& manager) : BasicBlockLevelPass(manager) {
    info_.name = "Silly move erasure";
    info_.repeat_while_changing = false;

    info_.preserve_ssa = true;
    info_.require_ssa = false;
  }

 protected:
  bool apply(IR::Function& function, IR::BasicBlock& block) override;
};
}  // namespace Passes
