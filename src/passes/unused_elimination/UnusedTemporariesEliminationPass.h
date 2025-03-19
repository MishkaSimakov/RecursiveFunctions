#pragma once
#include <unordered_set>

#include "intermediate_representation/Function.h"
#include "passes/pass_types/FunctionLevelPass.h"

namespace Passes {
class UnusedTemporariesEliminationPass : public FunctionLevelPass<> {
 public:
  UnusedTemporariesEliminationPass(PassManager& manager)
      : FunctionLevelPass(manager) {
    info_.name = "Unused temporaries elimination";
    info_.repeat_while_changing = true;

    info_.preserve_ssa = true;
    info_.require_ssa = false;
  }

 protected:
  bool apply(IR::Function& function) override;
};
}  // namespace Passes
