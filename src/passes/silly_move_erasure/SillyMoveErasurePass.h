#pragma once
#include "passes/Pass.h"

// After register allocation pass many virtual registers are mapped into one
// register. Sometimes instructions like
// "mov x0, x0" or "mov x0, x1; mov x1, x0" are generated afterwards.
// Aim of this pass is to remove instructions like this.

namespace Passes {
class SillyMoveErasurePass: public Pass {

 public:
  using Pass::Pass;

  void apply() override;
};
}  // namespace Passes
