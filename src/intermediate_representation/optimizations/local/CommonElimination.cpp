#include "CommonElimination.h"

void IR::CommonElimination::apply(BasicBlock& block) {
  // TODO: make this faster
  auto& instr = block.instructions;

  for (auto first = instr.begin(); first != instr.end(); ++first) {
    for (auto second = std::next(first); second != instr.end(); ++second) {
      if (**first != **second) {
        continue;
      }

      *second = std::make_unique<Move>((*second)->result_destination,
                                       (*first)->result_destination);
    }
  }
}
