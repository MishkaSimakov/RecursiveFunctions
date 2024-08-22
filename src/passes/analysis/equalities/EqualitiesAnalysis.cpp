#include "EqualitiesAnalysis.h"
void Passes::EqualitiesAnalysis::perform_analysis(const IR::Program& program) {
  equalities_.clear();

  for (auto& function : program.functions) {
    auto& function_equalities = equalities_[function.name];

    for (auto& block : function.basic_blocks) {
      for (auto& instruction : block.instructions) {
        if (instruction->is_of_type<IR::Move>()) {
          IR::Move move = static_cast<const IR::Move&>(*instruction);

          if (function_equalities.contains(move.arguments[0])) {
            move.arguments[0] = function_equalities.at(move.arguments[0]);
          }

          function_equalities.emplace(move.return_value, move.arguments[0]);
        }
      }
    }
  }
}
