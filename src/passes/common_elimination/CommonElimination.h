#pragma once
#include "passes/pass_types/RandomOrderPass.h"

namespace Passes {
class CommonElimination final : public RandomOrderPass {
 protected:
  void process_block(IR::Function&, IR::BasicBlock&) override;

 public:
  using RandomOrderPass::RandomOrderPass;
};
}  // namespace Passes
