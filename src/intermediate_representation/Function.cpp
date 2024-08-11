#include "Function.h"

std::unordered_set<IR::Value> IR::Function::calculate_escaping_recursively(
    BasicBlock* block, std::unordered_set<Value> used_above,
    std::unordered_set<const BasicBlock*>& used) {
  if (block == nullptr || used.contains(block)) {
    return {};
  }

  used.insert(block);

  std::unordered_set<Value> used_below;

  for (auto& instruction : block->instructions) {
    if (instruction->has_return_value()) {
      auto return_value = instruction->get_return_value();
      used_above.emplace(return_value);
      used_below.emplace(return_value);

      temporaries_info.emplace(return_value, TemporariesInfo{{block}, block});
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
    for (auto temporary :
         instruction->filter_arguments(ValueType::VIRTUAL_REGISTER)) {
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
    if (!is_recursive) {
      for (auto& instruction : block.instructions) {
        auto* call = dynamic_cast<const FunctionCall*>(instruction.get());

        if (call != nullptr && call->name == name) {
          is_recursive = true;
        }
      }
    }

    if (block.is_end()) {
      end_blocks.push_back(&block);
    }
  }
}

void IR::Function::calculate_escaping_temporaries() {
  for (auto argument : arguments) {
    temporaries_info.emplace(argument,
                             TemporariesInfo{{begin_block}, begin_block});
  }

  std::unordered_set<const BasicBlock*> used_blocks;
  calculate_escaping_recursively(begin_block, {}, used_blocks);
}

void IR::Function::replace_values(
    const std::unordered_map<Value, Value>& mapping) const {
  for (auto& block : basic_blocks) {
    for (auto& instruction : block.instructions) {
      instruction->replace_values(mapping);
    }
  }
}
