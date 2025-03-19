#pragma once
#include <unordered_set>

#include "intermediate_representation/Function.h"

namespace Passes {
class Spiller {
  static void process_function_call(
      const std::unordered_set<IR::Value>&, IR::Function&, IR::BasicBlock&,
      IR::BasicBlock::InstructionsListT::iterator);

 public:
  static void spill(IR::Function&, const std::unordered_set<IR::Value>&);
};
}  // namespace Passes
