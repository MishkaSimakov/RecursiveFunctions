#pragma once
#include <unordered_set>
#include <functional>

#include "intermediate_representation/BasicBlock.h"

namespace IR {
class Optimizer {
  void apply_to_each_block_recursively(
      BasicBlock* block, const auto& callable,
      std::unordered_set<BasicBlock*>& used) const {
    if (block == nullptr) {
      return;
    }

    if (used.contains(block)) {
      return;
    }

    used.insert(block);

    std::invoke(callable, *block);

    apply_to_each_block_recursively(block->children.first, callable, used);
    apply_to_each_block_recursively(block->children.second, callable, used);
  }

  void apply_to_each_block(Function& function, const auto& callable) const {
    std::unordered_set<BasicBlock*> used;
    apply_to_each_block_recursively(function.begin_block, callable, used);
  }

  void apply_to_each_block(Program& program, const auto& callable) const {
    for (auto& function : program.functions) {
      apply_to_each_block(function, callable);
    }
  }

 public:
  void apply(Program&);
};
}  // namespace IR
