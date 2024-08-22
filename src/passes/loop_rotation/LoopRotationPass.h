#pragma once
#include "passes/analysis/dominators/DominatorsAnalysis.h"
#include "passes/pass_types/FunctionLevelPass.h"

namespace Passes {
class LoopRotationPass : public FunctionLevelPass<> {
 public:
  LoopRotationPass(PassManager& manager)
      : FunctionLevelPass(manager, {"Loop rotation", false}) {}

 protected:
  bool apply(IR::Function& function) override;

  bool rotate_loop(IR::Function&, DominatorsAnalysis::Loop&);
};
}  // namespace Passes
