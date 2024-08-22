#include "UnusedTemporariesEliminationPass.h"

#include "passes/PassManager.h"
#include "passes/analysis/liveness/LivenessAnalysis.h"

bool Passes::UnusedTemporariesEliminationPass::apply(IR::Function& function) {
  bool was_changed = false;

  const auto& live =
      manager_.get_analysis<LivenessAnalysis>().get_liveness_info();

  for (auto& block : function.basic_blocks) {
    size_t count = std::erase_if(
        block.instructions,
        [&live](const std::unique_ptr<IR::BaseInstruction>& instruction) {
          if (!instruction->has_return_value()) {
            return false;
          }

          IR::Value return_value = instruction->get_return_value();
          auto& live_values = live.get_data(instruction.get(), Position::AFTER);
          return live_values.at(return_value) != TemporaryLivenessState::LIVE;
        });

    if (count != 0) {
      was_changed = true;
    }
  }

  return was_changed;
}
