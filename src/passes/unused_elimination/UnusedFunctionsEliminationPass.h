#pragma once
#include "intermediate_representation/Function.h"
#include "passes/pass_types/ModuleLevelPass.h"

namespace Passes {
class UnusedFunctionsEliminationPass : public ModuleLevelPass {
 public:
  UnusedFunctionsEliminationPass(PassManager& manager)
      : ModuleLevelPass(manager) {
    info_.name = "Unused functions elimination";
    info_.repeat_while_changing = true;

    info_.preserve_ssa = true;
    info_.require_ssa = false;
  }

 protected:
  std::unordered_set<std::string> used_;

  bool apply(IR::Program& program) override;

  void find_used_recursively(IR::Program&, const IR::Function&);
};
}  // namespace Passes
