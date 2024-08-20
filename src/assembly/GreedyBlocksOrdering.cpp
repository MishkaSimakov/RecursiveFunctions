#include "GreedyBlocksOrdering.h"

#include <unordered_set>

#include "intermediate_representation/Function.h"

const IR::BasicBlock* Assembly::GreedyBlocksOrdering::choose_next(
    const IR::BasicBlock* current,
    const std::unordered_set<const IR::BasicBlock*>& unvisited) const {
  for (auto child : current->children) {
    if (child != nullptr && unvisited.contains(child)) {
      return child;
    }
  }

  return *unvisited.begin();
}

std::vector<const IR::BasicBlock*> Assembly::GreedyBlocksOrdering::get_order(
    const IR::Function& function) const {
  std::vector<const IR::BasicBlock*> result;
  std::unordered_set<const IR::BasicBlock*> unvisited;

  for (auto& block : function.basic_blocks) {
    unvisited.insert(&block);
  }

  const IR::BasicBlock* current = function.begin_block;
  unvisited.erase(current);
  result.push_back(current);

  while (!unvisited.empty()) {
    current = choose_next(current, unvisited);

    unvisited.erase(current);
    result.push_back(current);
  }

  return result;
}
