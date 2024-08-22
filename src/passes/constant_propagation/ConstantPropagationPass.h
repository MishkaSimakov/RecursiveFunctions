#pragma once
#include "passes/pass_types/BasicBlockLevelPass.h"

namespace Passes {
class ConstantPropagationPass : public BasicBlockLevelPass<> {
 public:
  ConstantPropagationPass(PassManager& manager)
      : BasicBlockLevelPass(manager, {"Constant propagation", false}) {}

 protected:
  bool apply(IR::Function& function, IR::BasicBlock& block) override;
};
}  // namespace Passes
