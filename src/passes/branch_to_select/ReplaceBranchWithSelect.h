#pragma once
#include "intermediate_representation/BasicBlock.h"
#include "passes/Pass.h"
#include "passes/pass_types/FunctionLevelPass.h"

namespace Passes {
class ReplaceBranchWithSelect : public FunctionLevelPass<> {
  bool can_apply_to_phi(const IR::BasicBlock&, const IR::Phi&) const;

 public:
  ReplaceBranchWithSelect(PassManager& manager)
      : FunctionLevelPass(manager) {
    info_.name = "Replace branch with select";
    info_.repeat_while_changing = false;

    info_.preserve_ssa = true;
    info_.require_ssa = true;
  }

 protected:
  bool apply(IR::Function& function) override;
};
}  // namespace Passes
