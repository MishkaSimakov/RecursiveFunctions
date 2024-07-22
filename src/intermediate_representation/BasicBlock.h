#pragma once

#include <deque>
#include <list>
#include <memory>
#include <unordered_map>

#include "Instruction.h"

namespace IR {
struct BasicBlock {
  // instructions
  std::list<std::unique_ptr<Instruction>> instructions;

  // 2 children
  std::pair<BasicBlock*, BasicBlock*> children;

  // many parents
  std::vector<BasicBlock*> parents;

  bool is_begin() const { return parents.empty(); }

  bool is_end() const { return children.first == nullptr; }

  bool is_full() const { return !is_begin() && !is_end(); }
};

struct Function {
 private:
  void calculate_escaping_recursively(BasicBlock* block) {
    if (block == nullptr) {
      return;
    }

    for (auto& instruction : block->instructions) {
      if (instruction->has_return_value()) {
        temporaries_info.emplace(instruction->result_destination,
                                 TemporariesInfo{false, block});
      }

      for (auto temporary : instruction->get_temporaries_in_arguments()) {
        auto& info = temporaries_info[temporary];
        if (info.origin_block != block) {
          info.is_escaping = true;
        }
      }
    }

    calculate_escaping_recursively(block->children.first);
    calculate_escaping_recursively(block->children.second);
  }

  void calculate_end_blocks() {
    for (auto& block : basic_blocks) {
      if (block.is_end()) {
        end_blocks.push_back(&block);
      }
    }
  }

  void calculate_escaping_temporaries() {
    for (size_t i = 0; i < arguments_count; ++i) {
      temporaries_info.emplace(Temporary{i},
                               TemporariesInfo{false, begin_block});
    }

    calculate_escaping_recursively(begin_block);
  }

 public:
  static constexpr auto entrypoint = "main";

  struct TemporariesInfo {
    bool is_escaping;
    BasicBlock* origin_block;
  };

  std::string name;
  std::deque<BasicBlock> basic_blocks;
  BasicBlock* begin_block;
  size_t arguments_count;
  std::vector<BasicBlock*> end_blocks;

  // it is guaranteed that ALL temporaries are contained inside the variable
  std::unordered_map<Temporary, TemporariesInfo> temporaries_info;

  explicit Function(std::string name)
      : name(std::move(name)), begin_block(nullptr), arguments_count(0) {}

  Function(Function&&) = default;

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

  void finalize_function() {
    calculate_end_blocks();
    calculate_escaping_temporaries();
  }
};

struct Program {
  std::vector<Function> functions;
};
}  // namespace IR
