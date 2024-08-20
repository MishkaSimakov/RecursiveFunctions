#pragma once
#include "passes/pass_types/BasicBlockLevelPass.h"

namespace Passes {
class SSAMoveErasure
    : public BasicBlockLevelPass<ReversedPostBasicBlocksOrder> {
 public:
  using BasicBlockLevelPass::BasicBlockLevelPass;

 protected:
  std::unordered_map<IR::Value, IR::Value> replacements_;

  bool apply(IR::Function& function, IR::BasicBlock& block) override;
};
}  // namespace Passes
