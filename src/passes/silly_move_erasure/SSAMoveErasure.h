#pragma once
#include "passes/pass_types/BasicBlockLevelPass.h"

namespace Passes {
class SSAMoveErasure
    : public BasicBlockLevelPass<> {
 public:
  SSAMoveErasure(PassManager& manager)
      : BasicBlockLevelPass(manager, {"SSA move erasure", false}) {}

 protected:
  bool apply(IR::Function& function, IR::BasicBlock& block) override;
};
}  // namespace Passes
