#pragma once
#include "intermediate_representation/BasicBlock.h"
#include "passes/Pass.h"
#include "passes/pass_types/FunctionLevelPass.h"

namespace Passes {
class ReplaceBranchWithSelect : public FunctionLevelPass<> {
  bool can_apply_to_phi(const IR::BasicBlock&, const IR::Phi&) const;

 public:
  ReplaceBranchWithSelect(PassManager& manager)
      : FunctionLevelPass(manager, {"Replace branch with select", false}) {}

 protected:
  bool apply(IR::Function& function) override;
};
}  // namespace Passes
