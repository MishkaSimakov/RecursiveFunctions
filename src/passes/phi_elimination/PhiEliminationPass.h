#pragma once
#include "passes/pass_types/FunctionLevelPass.h"

namespace Passes {
class PhiEliminationPass : public FunctionLevelPass<> {
 public:
  PhiEliminationPass(PassManager& manager)
      : FunctionLevelPass(manager, {"Phi elimination", false}) {}

 protected:
  bool apply(IR::Function& function) override;
};
}  // namespace Passes
