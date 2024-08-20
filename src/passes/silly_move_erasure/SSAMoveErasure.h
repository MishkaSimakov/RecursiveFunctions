#pragma once
#include "passes/Pass.h"
#include "passes/pass_types/ParentsFirstPass.h"

namespace Passes {
class SSAMoveErasure : public ParentsFirstPass {
 public:
  using ParentsFirstPass::ParentsFirstPass;

 protected:
  std::unordered_map<IR::Value, IR::Value> replacements_;

  void process_block(IR::Function&, IR::BasicBlock&) override;
};
}  // namespace Passes
