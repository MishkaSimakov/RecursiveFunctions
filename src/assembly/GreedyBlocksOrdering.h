#pragma once
#include <unordered_set>
#include <vector>

#include "intermediate_representation/BasicBlock.h"

namespace Assembly {
class GreedyBlocksOrdering {
  const IR::BasicBlock* choose_next(
      const IR::BasicBlock* current,
      const std::unordered_set<const IR::BasicBlock*>& unvisited) const;

 public:
  std::vector<const IR::BasicBlock*> get_order(const IR::Function&) const;
};
}  // namespace Assembly
