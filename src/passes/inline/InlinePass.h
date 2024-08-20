#pragma once
#include "intermediate_representation/BasicBlock.h"
#include "intermediate_representation/Function.h"
#include "passes/pass_types/ModuleLevelPass.h"

namespace Passes {
class InlinePass : public ModuleLevelPass {
  void inline_function_call(IR::Function&, IR::BasicBlock&,
                            IR::BasicBlock::InstructionsListT::iterator);

 public:
  using ModuleLevelPass::ModuleLevelPass;

 protected:
  bool apply(IR::Program& program) override;
};
}  // namespace Passes
