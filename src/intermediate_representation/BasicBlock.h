#pragma once

#include <list>
#include <memory>

#include "Instruction.h"

namespace IR {
struct BasicBlock {
  // instructions
  std::list<std::unique_ptr<Instruction>> instructions;

  // condition (temporary condition always: left if zero, right otherwise)
  // condition or return value temporary
  TemporaryOrConstant end_value;

  // 2 children
  using shared_pointer = std::shared_ptr<BasicBlock>;
  std::pair<shared_pointer, shared_pointer> children;

  // many parents
  using weak_pointer = std::weak_ptr<BasicBlock>;
  std::vector<weak_pointer> parents;

  void append(BasicBlock other) {
    if (!is_end()) {
      throw std::runtime_error("In IR cannot append to non-end basic block");
    }

    if (!other.is_begin()) {
      throw std::runtime_error("In IR cannot append non-begin basic block");
    }

    instructions.splice(instructions.end(), other.instructions);
    end_value = other.end_value;
    children = std::move(other.children);
  }

  bool is_begin() const { return parents.empty(); }

  bool is_end() const { return children.first == nullptr; }

  bool is_full() const { return !is_begin() && !is_end(); }
};

struct Function {
  static constexpr auto entrypoint = "main";

  std::string name;
  std::shared_ptr<BasicBlock> begin_block;
};

struct Program {
  std::vector<Function> functions;
};
}  // namespace IR
