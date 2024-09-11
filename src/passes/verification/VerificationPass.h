#pragma once
#include "passes/pass_types/ModuleLevelPass.h"

namespace Passes {
class VerificationException final : public std::runtime_error {
public:
  const IR::Function& function;

  explicit VerificationException(const IR::Function& function,
                                 std::string string)
      : std::runtime_error(std::move(string)), function(function) {}
};

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
