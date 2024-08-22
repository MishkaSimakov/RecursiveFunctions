#pragma once

#include <algorithm>
#include <deque>
#include <list>
#include <memory>

#include "Instruction.h"

namespace IR {
struct Function;

struct BasicBlock {
  using InstructionsListT = std::list<std::unique_ptr<BaseInstruction>>;

  // instructions
  InstructionsListT instructions;

  // 2 children
  std::array<BasicBlock*, 2> children;

  // many parents
  std::vector<BasicBlock*> parents;

  std::span<BasicBlock*> nonnull_children() {
    return {children.begin(), children_count()};
  }
  std::span<BasicBlock* const> nonnull_children() const {
    return {children.begin(), children_count()};
  }

  size_t children_count() const {
    size_t cnt = 0;

    for (auto child : children) {
      if (child == nullptr) {
        break;
      }

      ++cnt;
    }

    return cnt;
  }

  bool is_begin() const { return parents.empty(); }

  bool is_end() const { return children[0] == nullptr; }

  bool is_full() const { return !is_begin() && !is_end(); }

  bool has_one_child() const { return children_count() == 1; }

  bool has_one_parent() const { return parents.size() == 1; }

  BasicBlock copy_instructions() const {
    BasicBlock copy;

    for (auto& instruction : instructions) {
      copy.instructions.push_back(instruction->clone());
    }

    return copy;
  }
};
}  // namespace IR
