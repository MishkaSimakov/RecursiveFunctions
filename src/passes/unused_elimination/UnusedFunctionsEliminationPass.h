#pragma once
#include "intermediate_representation/Function.h"
#include "passes/Pass.h"

namespace Passes {
class UnusedFunctionsEliminationPass : public Pass {
 public:
  using Pass::Pass;

  void apply() override;

 private:
  std::unordered_set<std::string> used_;

  void find_used_recursively(const IR::Function&);
};
}  // namespace Passes
