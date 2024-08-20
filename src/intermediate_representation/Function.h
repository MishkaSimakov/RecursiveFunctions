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
  template <typename T, typename F>
    requires std::same_as<std::decay_t<T>, Function>
  static void reversed_postorder_traversal_helper(T& function, F&& callable) {
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

  void simplify_blocks_recursive(BasicBlock*, std::unordered_set<const BasicBlock*>&);

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

  // list because in inline pass pointers must not be invalidated after erase
  std::list<BasicBlock> basic_blocks;
  BasicBlock* begin_block;
  std::vector<Value> arguments;
  std::vector<BasicBlock*> end_blocks;

  std::unordered_set<std::string> calls;

  // it is guaranteed that ALL temporaries are contained inside the variable
  std::unordered_set<Value> temporaries;

  explicit Function(std::string name)
      : name(std::move(name)), begin_block(nullptr) {}

  Function(Function&&) = default;
  Function& operator=(Function&&) = default;

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

  Value allocate_vreg();

  size_t get_max_temporary_index() const {
    size_t temporary_index = 0;
    for (const auto& temp : temporaries) {
      if (temp.value > temporary_index) {
        temporary_index = temp.value;
      }
    }

    return temporary_index;
  }

  void finalize();

  void simplify_blocks();

  bool is_recursive() const { return calls.contains(name); }

  bool is_leaf() const { return calls.empty(); }

  void replace_phi_parents(
      const std::unordered_map<const BasicBlock*, BasicBlock*>&);

  // traverse all basic blocks in such order that when one block is calculated
  template <typename F>
    requires std::invocable<F, BasicBlock*>
  void reversed_postorder_traversal(F&& callable) {
    reversed_postorder_traversal_helper(*this, callable);
  }

  template <typename F>
    requires std::invocable<F, const BasicBlock*>
  void reversed_postorder_traversal(F&& callable) const {
    reversed_postorder_traversal_helper(*this, callable);
  }

  size_t size() const {
    size_t result = 0;
    for (auto& block : basic_blocks) {
      result += block.instructions.size();
    }

    return result;
  }
};
}  // namespace IR
