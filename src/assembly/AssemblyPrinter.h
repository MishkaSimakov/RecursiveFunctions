#pragma once
#include <list>
#include <string>
#include <vector>

#include "intermediate_representation/Program.h"

namespace Assembly {
struct InstructionContext {
  std::unordered_map<const IR::BasicBlock*, std::string> labels;
  std::vector<const IR::BasicBlock*> ordering;

  const IR::Function& function;
  size_t block_index;

  std::vector<IR::Value> callee_saved_registers;

  explicit InstructionContext(const IR::Function& function)
      : function(function), block_index(0) {}

  size_t get_block_index(const IR::BasicBlock* block) const {
    auto itr = std::ranges::find(ordering, block);
    return itr - ordering.begin();
  }

  size_t is_next(const IR::BasicBlock* block) const {
    if (block_index + 1 == ordering.size()) {
      return false;
    }

    return ordering[block_index + 1] == block;
  }
};

class AssemblyPrinter {
  const IR::Program& program_;
  std::vector<std::string> result;

  void before_function(const InstructionContext&);
  void after_function(const InstructionContext&);

 public:
  explicit AssemblyPrinter(const IR::Program& program) : program_(program) {}

  static std::string mangle_function_name(const IR::Function&);
  static std::string mangle_function_name(const std::string&);

  static std::string print_value(IR::Value);

  std::vector<std::string> print();
};
}  // namespace Assembly
