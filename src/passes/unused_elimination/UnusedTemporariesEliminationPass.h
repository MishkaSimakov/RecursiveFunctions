#pragma once
#include <unordered_set>

#include "intermediate_representation/Function.h"
#include "passes/pass_types/FunctionLevelPass.h"

namespace Passes {
class UnusedTemporariesEliminationPass : public FunctionLevelPass<> {
 public:
  UnusedTemporariesEliminationPass(PassManager& manager)
      : FunctionLevelPass(manager, {"Unused temporaries elimination", true}) {}

 protected:
  bool apply(IR::Function& function) override;
};
}  // namespace Passes
