#include "CommonElimination.h"

void IR::CommonElimination::apply(BasicBlock& block) {
  // TODO: make this faster
  auto& instr = block.instructions;

  for (auto first = instr.begin(); first != instr.end(); ++first) {
    for (auto second = std::next(first); second != instr.end(); ++second) {
      if (**first != **second) {
        continue;
      }

      auto move_instruction = std::make_unique<Move>();
      move_instruction->result_destination = (*second)->result_destination;
      move_instruction->source = (*first)->result_destination;

      *second = std::move(move_instruction);
    }
  }
}
