#pragma once
#include "passes/pass_types/BasicBlockLevelPass.h"

namespace Passes {
class CommonElimination final : public BasicBlockLevelPass<> {
 public:
  CommonElimination(PassManager& manager)
      : BasicBlockLevelPass(manager,
                            {"Common expressions elimination", false}) {}

 protected:
  bool apply(IR::Function& function, IR::BasicBlock& block) override;
};
}  // namespace Passes
