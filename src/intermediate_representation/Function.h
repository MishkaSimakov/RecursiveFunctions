#pragma once

#include <deque>
#include <functional>
#include <unordered_set>

#include "BasicBlock.h"
#include "Temporary.h"

namespace IR {
struct Function {
 private:
  std::unordered_set<Temporary> calculate_escaping_recursively(
      BasicBlock* block, std::unordered_set<Temporary> used_above,
      std::unordered_set<const BasicBlock*>& used);

  void calculate_end_blocks();

  void calculate_escaping_temporaries();

 public:
  static constexpr auto entrypoint = "main";

  struct TemporariesInfo {
    // this is basic blocks that use that temporary or blocks which ancestors
    // use that temporary
    std::unordered_set<const BasicBlock*> using_blocks;

    // block where temporary is first (and last because of ssa) assigned
    BasicBlock* origin_block;

    bool is_escaping() const { return is_used_in_descendants(origin_block); }

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

  // traverse all basic blocks in such order that when one block is calculated
  void traverse_blocks(std::function<void(const BasicBlock*)> callable) const;
};
}  // namespace IR
