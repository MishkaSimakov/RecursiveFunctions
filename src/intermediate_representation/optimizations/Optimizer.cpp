#include "Optimizer.h"

#include "local/CommonElimination.h"

void IR::Optimizer::apply(Program& program) {
  apply_to_each_block(
      program, [](BasicBlock& block) { CommonElimination().apply(block); });
}
