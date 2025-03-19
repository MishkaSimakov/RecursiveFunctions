#pragma once
#include <unordered_map>

#include "intermediate_representation/BasicBlock.h"
#include "intermediate_representation/Instruction.h"

namespace Passes {
enum class Position { BEFORE, AFTER };

template <typename T>
struct DFAStorage {
  using DataT = T;

  std::unordered_map<std::pair<const IR::BaseInstruction*, Position>, T> data;

  T& get_data(const IR::BaseInstruction* instruction, Position position) {
    return data[std::pair{instruction, position}];
  }

  T& get_data(const IR::BasicBlock* block, Position position) {
    if (position == Position::BEFORE) {
      return get_data(block->instructions.front().get(), Position::BEFORE);
    }

    return get_data(block->instructions.back().get(), Position::AFTER);
  }

  const T& get_data(const IR::BaseInstruction* instruction,
                    Position position) const {
    return data.at(std::pair{instruction, position});
  }

  const T& get_data(const IR::BasicBlock* block, Position position) const {
    if (position == Position::BEFORE) {
      return get_data(block->instructions.front().get(), Position::BEFORE);
    }

    return get_data(block->instructions.back().get(), Position::AFTER);
  }

  void clear() { data.clear(); }
};
}  // namespace Passes
