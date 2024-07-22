#pragma once

#include <deque>
#include <list>
#include <memory>
#include <unordered_map>
#include <unordered_set>

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
  std::unordered_set<Temporary> calculate_escaping_recursively(
      BasicBlock* block, std::unordered_set<Temporary> used_above) {
    if (block == nullptr) {
      return {};
    }

    std::unordered_set<Temporary> used_below;

    for (auto& instruction : block->instructions) {
      if (instruction->has_return_value()) {
        used_above.emplace(instruction->result_destination);
        used_below.emplace(instruction->result_destination);

        temporaries_info.emplace(instruction->result_destination,
                                 TemporariesInfo{{block}, block});
      }
    }

    auto left_used =
        calculate_escaping_recursively(block->children.first, used_above);
    auto right_used =
        calculate_escaping_recursively(block->children.second, used_above);

    used_below.insert(left_used.begin(), left_used.end());
    used_below.insert(right_used.begin(), right_used.end());

    for (auto& instruction : block->instructions) {
      for (auto temporary : instruction->get_temporaries_in_arguments()) {
        used_below.insert(temporary);
      }
    }

    for (auto temporary : used_below) {
      temporaries_info[temporary].using_blocks.insert(block);
    }

    return used_below;
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
                               TemporariesInfo{{begin_block}, begin_block});
    }

    calculate_escaping_recursively(begin_block, {});
  }

 public:
  static constexpr auto entrypoint = "main";

  struct TemporariesInfo {
    // this is basic blocks that use that temporary or blocks which ancestors
    // use that temporary
    std::unordered_set<const BasicBlock*> using_blocks;

    // block where temporary is first (and last because of ssa) assigned
    BasicBlock* origin_block;

    bool is_escaping() const { return using_blocks.size() > 1; }

    bool is_used_in_descendants(const BasicBlock* block) const {
      if (using_blocks.contains(block->children.first)) {
        return true;
      }

      if (using_blocks.contains(block->children.second)) {
        return true;
      }

      return false;
    }
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

  void finalize() {
    calculate_end_blocks();
    calculate_escaping_temporaries();
  }
};

struct Program {
  std::vector<Function> functions;
};
}  // namespace IR
