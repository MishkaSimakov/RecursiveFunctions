#pragma once

#include <deque>
#include <list>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "Instruction.h"

namespace IR {
struct Function;

struct BasicBlock {
  // instructions
  std::list<std::unique_ptr<Instruction>> instructions;

  // 2 children
  std::array<BasicBlock*, 2> children;

  // many parents
  std::vector<BasicBlock*> parents;

  bool is_begin() const { return parents.empty(); }

  bool is_end() const { return children[0] == nullptr; }

  bool is_full() const { return !is_begin() && !is_end(); }
};

struct Program {
  std::vector<Function> functions;
};
}  // namespace IR
