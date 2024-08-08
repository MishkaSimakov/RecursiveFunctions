#pragma once
#include <list>
#include <string>
#include <vector>

#include "intermediate_representation/Program.h"

namespace Assembly {
struct InstructionContext {
  const std::unordered_map<const IR::BasicBlock*, std::string>& labels;
  const std::vector<const IR::BasicBlock*>& ordering;

  size_t block_index;

  size_t get_block_index(const IR::BasicBlock* block) const {
    auto itr = std::ranges::find(ordering, block);
    return itr - ordering.begin();
  }
};

class AssemblyPrinter {
  const IR::Program& program_;
  std::vector<std::string> result;

  void before_function(const IR::Function&);
  void after_function(const IR::Function&);

 public:
  explicit AssemblyPrinter(const IR::Program& program) : program_(program) {}

  static std::string mangle_function_name(const IR::Function&);
  static std::string mangle_function_name(const std::string&);

  std::vector<std::string> print();
};
}  // namespace Assembly
