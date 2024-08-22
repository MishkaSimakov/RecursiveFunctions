#pragma once
#include "passes/pass_types/ModuleLevelPass.h"

namespace Passes {
class VerificationPass : public ModuleLevelPass {
 public:
  VerificationPass(PassManager& manager) : ModuleLevelPass(manager) {
    info_.name = "Verification pass";
    info_.repeat_while_changing = false;

    info_.preserve_ssa = true;
    info_.require_ssa = false;
  }

 protected:
  bool apply(IR::Program& program) override;

  void check_ssa(const IR::Function&);
};
}  // namespace Passes
