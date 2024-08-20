#pragma once
#include <span>
#include <unordered_set>

#include "intermediate_representation/Function.h"
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

template <typename T>
class ForwardDFA {
 private:
  bool transfer_through_block(const IR::BasicBlock& block);

 protected:
  using DFAValueT = T;

  DFAStorage<T> storage_;

  virtual T meet(std::span<T> parents) const = 0;
  virtual T transfer(const T& before,
                     const IR::BaseInstruction& instruction) const = 0;

  virtual void init() = 0;

  void start_dfa(const IR::Function& function);

 public:
  virtual ~ForwardDFA() = default;
};

template <typename T>
bool ForwardDFA<T>::transfer_through_block(const IR::BasicBlock& block) {
  for (auto itr = block.instructions.begin(); itr != block.instructions.end();
       ++itr) {
    const auto& before = storage_.get_data(itr->get(), Position::BEFORE);
    auto& after = storage_.get_data(itr->get(), Position::AFTER);

    auto new_result = transfer(before, **itr);

    if (new_result == after) {
      return false;
    }

    after = new_result;

    auto next = std::next(itr);
    if (next != block.instructions.end()) {
      storage_.get_data(next->get(), Position::BEFORE) = after;
    }
  }

  return true;
}

template <typename T>
void ForwardDFA<T>::start_dfa(const IR::Function& function) {
  storage_.clear();
  init();

  std::unordered_set<const IR::BasicBlock*> worklist;
  worklist.insert(function.begin_block);

  while (!worklist.empty()) {
    const IR::BasicBlock* current = *worklist.begin();
    worklist.erase(worklist.begin());

    std::span parents_data =
        current->parents |
        std::views::transform([this](const IR::BasicBlock* parent) {
          return storage_.get_data(parent, Position::AFTER);
        });

    auto meet_result = meet(parents_data);
    auto& before = storage_.get_data(current, Position::BEFORE);

    if (meet_result == before) {
      continue;
    }

    before = meet_result;

    bool was_changed = transfer_through_block(current);

    if (!was_changed) {
      continue;
    }

    for (auto child : current->children) {
      worklist.insert(child);
    }
  }
}
}  // namespace Passes
