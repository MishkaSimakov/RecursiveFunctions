#pragma once

#include <deque>
#include <list>

#include "Instruction.h"

namespace IR {
struct BasicBlock {
  // instructions
  std::list<std::unique_ptr<Instruction>> instructions;

  // condition (temporary condition always: left if zero, right otherwise)
  // condition or return value temporary
  TemporaryOrConstant end_value = TemporaryOrConstant::constant(0);

  // 2 children
  std::pair<BasicBlock*, BasicBlock*> children;

  // many parents
  std::vector<BasicBlock*> parents;

  bool is_begin() const { return parents.empty(); }

  bool is_end() const { return children.first == nullptr; }

  bool is_full() const { return !is_begin() && !is_end(); }
};

struct Function {
  static constexpr auto entrypoint = "main";

  std::string name;
  std::deque<BasicBlock> basic_blocks;
  BasicBlock* begin_block;

  Function(std::string name) : name(std::move(name)), begin_block(nullptr) {}

  template <typename... Args>
  BasicBlock* set_begin_block(Args&&... args) {
    begin_block = add_block(std::forward<Args>(args)...);
    return begin_block;
  }

  template <typename... Args>
  BasicBlock* add_block(Args&&... args) {
    basic_blocks.emplace_back(std::forward<Args>(args)...);
    return &basic_blocks.back();
  }
};

struct Program {
  std::vector<Function> functions;
};
}  // namespace IR
