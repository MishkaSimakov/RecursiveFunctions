#include "SSAMoveErasure.h"

#include "passes/PassManager.h"
#include "passes/analysis/equalities/EqualitiesAnalysis.h"

bool Passes::SSAMoveErasure::apply(IR::Function& function,
                                   IR::BasicBlock& block) {
  bool was_changed = false;
  auto& equalities =
      manager_.get_analysis<EqualitiesAnalysis>().get_equalities(function);

  for (auto& instruction : block.instructions) {
    if (instruction->is_of_type<IR::Move>()) {
      continue;
    }

    // TODO: move to some other pass
    if (instruction->is_of_type<IR::Phi>()) {
      auto& phi = static_cast<const IR::Phi&>(*instruction);

      // if (phi.parents.size() == 1) {
      //   instruction = std::make_unique<IR::Move>(phi.return_value,
      //                                            phi.parents.front().second);
      //
      //   was_changed = true;
      // }
    }

    if (instruction->replace_values(equalities)) {
      was_changed = true;
    }
  }

  return was_changed;
}
