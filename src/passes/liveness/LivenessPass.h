#pragma once
#include "LiveTemporariesStorage.h"
#include "passes/PassManager.h"
#include "passes/pass_types/ParentsFirstPass.h"

namespace Passes {
class LivenessPass : public ParentsFirstPass {
 protected:
  using Position = LiveTemporariesStorage::Position;

  void process_block(IR::Function&, IR::BasicBlock&) override;

  void before_function(IR::Function&) override;

  LiveTemporariesStorage& storage() const { return manager_.live_storage; }

 public:
  using ParentsFirstPass::ParentsFirstPass;
};
}  // namespace Passes
