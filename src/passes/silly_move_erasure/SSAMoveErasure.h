#pragma once
#include "passes/pass_types/BasicBlockLevelPass.h"

namespace Passes {
class SSAMoveErasure : public BasicBlockLevelPass<> {
 public:
  SSAMoveErasure(PassManager& manager) : BasicBlockLevelPass(manager) {
    info_.name = "SSA move erasure";
    info_.repeat_while_changing = false;

    info_.preserve_ssa = true;
    info_.require_ssa = true;
  }

 protected:
  bool apply(IR::Function& function, IR::BasicBlock& block) override;
};
}  // namespace Passes
