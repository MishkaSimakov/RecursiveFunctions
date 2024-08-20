#pragma once
#include "intermediate_representation/Function.h"
#include "passes/pass_types/ModuleLevelPass.h"

namespace Passes {
class UnusedFunctionsEliminationPass : public ModuleLevelPass {
 public:
  using ModuleLevelPass::ModuleLevelPass;

 protected:
  std::unordered_set<std::string> used_;

  bool apply(IR::Program& program) override;

  void find_used_recursively(const IR::Function&);
};
}  // namespace Passes
