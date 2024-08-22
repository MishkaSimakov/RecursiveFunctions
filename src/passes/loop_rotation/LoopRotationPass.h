#pragma once
#include "passes/analysis/dominators/DominatorsAnalysis.h"
#include "passes/pass_types/FunctionLevelPass.h"

namespace Passes {
class LoopRotationPass : public FunctionLevelPass<> {
 public:
  LoopRotationPass(PassManager& manager) : FunctionLevelPass(manager) {
    info_.name = "Loop rotation";
    info_.repeat_while_changing = false;

    info_.preserve_ssa = true;
    info_.require_ssa = true;
  }

 protected:
  bool apply(IR::Function& function) override;

  bool rotate_loop(IR::Function&, DominatorsAnalysis::Loop&);
};
}  // namespace Passes
