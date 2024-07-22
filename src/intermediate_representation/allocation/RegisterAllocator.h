#pragma once
#include "TemporaryDependenciesGraph.h"
#include "intermediate_representation/BasicBlock.h"

namespace IR {
// We should:
// 0. Find escaping variables
// 1. Calculate live variables at each instruction
// 2. Build TemporaryDependenciesGraph
// 3. Usgin some A* find graph colouring
// Profit!

class RegisterAllocator {
  constexpr static size_t kRegistersCount = 10;

  void apply_to_function(Function&);
public:
  void apply(Program&);
};
}
