#pragma once
#include "passes/pass_types/ModuleLevelPass.h"

namespace Passes {
class VerificationPass : public ModuleLevelPass {
 public:
  VerificationPass(PassManager& manager)
      : ModuleLevelPass(manager, {"Verification pass", false}) {}

 protected:
  bool apply(IR::Program& program) override;
};
}  // namespace Passes
