#pragma once
#include "intermediate_representation/BasicBlock.h"
#include "intermediate_representation/Function.h"
#include "passes/Pass.h"

namespace Passes {
class InlinePass : public Pass {
  void inline_function_call(IR::Function&, IR::BasicBlock&,
                            IR::BasicBlock::InstructionsListT::iterator);

 public:
  using Pass::Pass;

  void apply() override;
};
}  // namespace Passes
