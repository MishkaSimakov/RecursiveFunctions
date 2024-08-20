#include "SSAMoveErasure.h"

#include "passes/PassManager.h"

void Passes::SSAMoveErasure::process_block(IR::Function& function,
                                           IR::BasicBlock& block) {
  replacements_.clear();

  for (auto& instruction : block.instructions) {
    instruction->replace_values(replacements_);

    if (instruction->is_of_type<IR::Move>()) {
      auto& move = static_cast<const IR::Move&>(*instruction);

      if (move.arguments[0].is_temporary()) {
        replacements_.emplace(move.return_value, move.arguments[0]);
      }
    }
  }
}
