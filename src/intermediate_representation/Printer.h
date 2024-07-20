#pragma once

#include <iostream>
#include <unordered_map>

#include "BasicBlock.h"

namespace IR {

class Printer {
  static constexpr auto prefix = "\t";

  using UsedContainerT = std::unordered_map<const BasicBlock*, std::pair<size_t, bool>>;

  std::ostream& os_;
  UsedContainerT used_;

  void print_basic_block(const BasicBlock& basic_block);

  void print_basic_blocks_recursively(const BasicBlock& basic_block);

  size_t get_block_index(const BasicBlock& basic_block);

 public:
  explicit Printer(std::ostream& os = std::cout) : os_(os) {}

  void print(const Program& program);
};

}  // namespace IR
