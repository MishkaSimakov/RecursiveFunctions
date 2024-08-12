#pragma once

#include <functional>
#include <list>
#include <stack>
#include <unordered_set>

#include "BasicBlock.h"
#include "Value.h"

namespace IR {
struct Function {
 private:
  std::unordered_set<Value> calculate_escaping_recursively(
      BasicBlock* block, std::unordered_set<Value> used_above,
      std::unordered_set<const BasicBlock*>& used);

  void calculate_end_blocks();

  void calculate_escaping_temporaries();

  template <typename T, typename F>
    requires std::same_as<std::decay_t<T>, Function>
  static void postorder_traversal_helper(T& function, F&& callable) {
    using BB =
        std::conditional_t<std::is_const_v<T>, const BasicBlock*, BasicBlock*>;

    std::stack<BB> blocks_to_process;
    std::unordered_set<BB> visited;

    for (auto block : function.end_blocks) {
      visited.insert(block);
      blocks_to_process.push(block);
    }

    while (!blocks_to_process.empty()) {
      BB block = blocks_to_process.top();

      bool ready = true;
      for (auto parent : block->parents) {
        if (!visited.contains(parent)) {
          blocks_to_process.push(parent);
          visited.insert(parent);

          ready = false;
        }
      }

      if (!ready) {
        continue;
      }

      blocks_to_process.pop();
      callable(block);

      for (auto child : block->children) {
        if (child == nullptr) {
          continue;
        }

        if (!visited.contains(child)) {
          blocks_to_process.push(child);
          visited.insert(child);
        }
      }
    }
  }

  //
  size_t index_{0};

  void tie_to_program(size_t index) { index_ = index; }

  friend struct Program;

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
      for (auto child : block->children) {
        if (using_blocks.contains(child)) {
          return true;
        }
      }

      return false;
    }
  };

  std::string name;
  std::deque<BasicBlock> basic_blocks;
  BasicBlock* begin_block;
  std::vector<Value> arguments;
  std::vector<BasicBlock*> end_blocks;
  bool is_recursive{false};

  // it is guaranteed that ALL temporaries are contained inside the variable
  std::unordered_map<Value, TemporariesInfo> temporaries_info;

  explicit Function(std::string name)
      : name(std::move(name)), begin_block(nullptr) {}

  Function(Function&&) = default;

  void replace_values(const std::unordered_map<Value, Value>&) const;

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

  size_t get_max_temporary_index() const {
    size_t temporary_index = 0;
    for (const auto& temp : temporaries_info | std::views::keys) {
      if (temp.value > temporary_index) {
        temporary_index = temp.value;
      }
    }

    return temporary_index;
  }

  void finalize() {
    temporaries_info.clear();
    end_blocks.clear();
    is_recursive = false;

    calculate_end_blocks();
    calculate_escaping_temporaries();
  }

  // traverse all basic blocks in such order that when one block is calculated
  template <typename F>
    requires std::invocable<F, BasicBlock*>
  void postorder_traversal(F&& callable) {
    postorder_traversal_helper(*this, callable);
  }

  template <typename F>
    requires std::invocable<F, const BasicBlock*>
  void postorder_traversal(F&& callable) const {
    postorder_traversal_helper(*this, callable);
  }
};
}  // namespace IR
