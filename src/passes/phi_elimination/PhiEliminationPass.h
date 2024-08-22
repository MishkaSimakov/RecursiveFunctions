#pragma once
#include "passes/pass_types/FunctionLevelPass.h"

namespace Passes {
class PhiEliminationPass : public FunctionLevelPass<> {
 public:
  PhiEliminationPass(PassManager& manager)
    : FunctionLevelPass(manager) {
    info_.name = "Phi elimination";
    info_.repeat_while_changing = false;

    info_.preserve_ssa = false;
    info_.require_ssa = true;
  }

 protected:
  bool apply(IR::Function& function) override;
};
}  // namespace Passes
