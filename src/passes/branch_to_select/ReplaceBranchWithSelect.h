#pragma once
#include "intermediate_representation/BasicBlock.h"
#include "passes/Pass.h"

namespace Passes {
class ReplaceBranchWithSelect : public Pass {
  bool can_apply_to_phi(const IR::BasicBlock&, const IR::Phi&) const;

 public:
  using Pass::Pass;

  void apply() override;
};
}  // namespace Passes
