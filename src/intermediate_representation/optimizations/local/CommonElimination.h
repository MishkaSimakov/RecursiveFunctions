#pragma once
#include "intermediate_representation/BasicBlock.h"

namespace IR {
class CommonElimination {
 public:
  void apply(BasicBlock&);
};
}  // namespace IR
