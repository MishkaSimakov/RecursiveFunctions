#pragma once
#include "passes/pass_types/BasicBlockLevelPass.h"

namespace Passes {
class CommonElimination final : public BasicBlockLevelPass<> {
 public:
  using BasicBlockLevelPass::BasicBlockLevelPass;

 protected:
  bool apply(IR::Function& function, IR::BasicBlock& block) override;
};
}  // namespace Passes
