#include "Function.h"

#include <iostream>
#include <stack>

std::unordered_set<IR::Temporary> IR::Function::calculate_escaping_recursively(
    BasicBlock* block, std::unordered_set<Temporary> used_above,
    std::unordered_set<const BasicBlock*>& used) {
  if (block == nullptr || used.contains(block)) {
    return {};
  }

  used.insert(block);

  std::unordered_set<Temporary> used_below;

  for (auto& instruction : block->instructions) {
    if (instruction->has_return_value()) {
      used_above.emplace(instruction->result_destination);
      used_below.emplace(instruction->result_destination);

      temporaries_info.emplace(instruction->result_destination,
                               TemporariesInfo{{block}, block});
    }
  }

  for (auto child : block->children) {
    // we also calculate parents at the same time
    if (child != nullptr) {
      child->parents.push_back(block);
    }

    auto used_below_in_child =
        calculate_escaping_recursively(child, used_above, used);
    used_below.insert(used_below_in_child.begin(), used_below_in_child.end());
  }

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

void IR::Function::calculate_end_blocks() {
  for (auto& block : basic_blocks) {
    if (block.is_end()) {
      end_blocks.push_back(&block);
    }
  }
}

void IR::Function::calculate_escaping_temporaries() {
  for (size_t i = 0; i < arguments_count; ++i) {
    temporaries_info.emplace(Temporary{i},
                             TemporariesInfo{{begin_block}, begin_block});
  }

  std::unordered_set<const BasicBlock*> used_blocks;
  calculate_escaping_recursively(begin_block, {}, used_blocks);
}

void IR::Function::traverse_blocks(
    std::function<void(const BasicBlock*)> callable) const {
  std::stack<const BasicBlock*> blocks_to_process;
  std::unordered_set<const BasicBlock*> visited;

  for (auto block : end_blocks) {
    visited.insert(block);
    blocks_to_process.push(block);
  }

  while (!blocks_to_process.empty()) {
    const BasicBlock* block = blocks_to_process.top();

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
