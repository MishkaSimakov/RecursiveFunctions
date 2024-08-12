#pragma once
#include <unordered_set>

#include "intermediate_representation/Function.h"
#include "passes/Pass.h"

namespace Passes {
class UnusedTemporariesEliminationPass : public Pass {
 public:
  using Pass::Pass;

  void apply() override;

 private:
  std::unordered_set<IR::Value> find_used_in_function(
      const IR::Function&) const;

  void mark_used_recursively(IR::Value, const std::vector<std::vector<IR::Value>>&, std::unordered_set<IR::Value>&) const;
};
}  // namespace Passes
