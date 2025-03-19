#include "ConstantPropagationPass.h"

#include "InstructionValueCalculator.h"

bool Passes::ConstantPropagationPass::apply(IR::Function& function,
                                            IR::BasicBlock& block) {
  bool was_changed = false;

  for (auto& instruction : block.instructions) {
    auto value = InstructionValueCalculator().calculate(*instruction);

    if (value.has_value() && instruction->has_return_value()) {
      was_changed = true;

      instruction = std::make_unique<IR::Move>(instruction->get_return_value(),
                                               value.value());
    }
  }

  return was_changed;
}
