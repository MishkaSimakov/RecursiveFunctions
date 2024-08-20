#pragma once
#include "intermediate_representation/Function.h"
#include "passes/Pass.h"

// This pass now only works with recursion generated by Recursive Functions
// language compiler. Moreover it does not support argmin inside recursive
// function
namespace Passes {
class RecursionToLoopPass : public Pass {
 private:
  bool is_suitable(const IR::Function&) const;

  bool check_recursive_calls_count(const IR::Function&, const IR::BasicBlock&, size_t) const;

 public:
  using Pass::Pass;

  void apply() override;
};
}  // namespace Passes
