#pragma once
#include "passes/pass_types/FunctionLevelPass.h"

namespace Passes {
class PhiEliminationPass : public FunctionLevelPass<> {
 public:
  using FunctionLevelPass::FunctionLevelPass;

 protected:
  bool apply(IR::Function& function) override;
};
}  // namespace Passes
