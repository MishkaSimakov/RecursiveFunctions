#include "Function.h"

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

  auto left_used =
      calculate_escaping_recursively(block->children.first, used_above, used);
  auto right_used =
      calculate_escaping_recursively(block->children.second, used_above, used);

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
  enum class BasicBlockStatus { DEFAULT, IN_QUEUE, PROCESSED };

  std::stack<const BasicBlock*> blocks_to_process;
  std::unordered_map<const BasicBlock*, BasicBlockStatus> statuses;

  for (auto block : end_blocks) {
    statuses[block] = BasicBlockStatus::IN_QUEUE;
    blocks_to_process.push(block);
  }

  while (!blocks_to_process.empty()) {
    const BasicBlock* block = blocks_to_process.top();

    bool ready = true;
    for (auto parent : block->parents) {
      BasicBlockStatus& status = statuses[parent];

      if (status == BasicBlockStatus::DEFAULT) {
        blocks_to_process.push(parent);
        status = BasicBlockStatus::IN_QUEUE;
      }

      if (status != BasicBlockStatus::PROCESSED) {
        ready = false;
      }
    }

    if (!ready) {
      continue;
    }

    blocks_to_process.pop();
    callable(block);
    statuses[block] = BasicBlockStatus::PROCESSED;
  }
}
