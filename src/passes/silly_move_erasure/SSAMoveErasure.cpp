#include "SSAMoveErasure.h"

#include "passes/PassManager.h"

bool Passes::SSAMoveErasure::apply(IR::Function& function,
                                   IR::BasicBlock& block) {
  bool was_changed = false;
  replacements_.clear();

  for (auto& instruction : block.instructions) {
    if (instruction->replace_values(replacements_)) {
      was_changed = true;
    }

    if (instruction->is_of_type<IR::Move>()) {
      auto& move = static_cast<const IR::Move&>(*instruction);

      if (move.arguments[0].is_temporary()) {
        replacements_.emplace(move.return_value, move.arguments[0]);
      }
    }
  }

  return was_changed;
}
