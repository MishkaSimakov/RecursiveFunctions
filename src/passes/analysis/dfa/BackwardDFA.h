#pragma once
#include <iostream>
#include <unordered_set>

#include "DFAStorage.h"
#include "intermediate_representation/Function.h"
#include "intermediate_representation/Instruction.h"

// TODO: unite this with ForwardDFA somehow...
namespace Passes {
template <typename T>
class BackwardDFA {
  bool transfer_through_block(const IR::BasicBlock& block,
                              bool stop_on_unchanged);

  void process_one_block(const IR::BasicBlock& block,
                         std::unordered_set<const IR::BasicBlock*>& worklist,
                         bool stop_on_unchanged);

 protected:
  using DFAValueT = T;

  DFAStorage<T> storage_;

  virtual T meet(std::vector<const T*> children) const = 0;
  virtual T transfer(const T& after,
                     const IR::BaseInstruction& instruction) const = 0;
  virtual void init(const IR::Function&) = 0;

  void start_dfa(const IR::Program& program);

 public:
  virtual ~BackwardDFA() = default;
};

template <typename T>
bool BackwardDFA<T>::transfer_through_block(const IR::BasicBlock& block,
                                            bool stop_on_unchanged) {
  for (auto itr = block.instructions.rbegin(); itr != block.instructions.rend();
       ++itr) {
    auto& before = storage_.get_data(itr->get(), Position::BEFORE);
    const auto& after = storage_.get_data(itr->get(), Position::AFTER);

    auto new_result = transfer(after, **itr);

    if (new_result == before && stop_on_unchanged) {
      return false;
    }

    before = new_result;

    auto next = std::next(itr);
    if (next != block.instructions.rend()) {
      storage_.get_data(next->get(), Position::AFTER) = before;
    }
  }

  return true;
}
template <typename T>
void BackwardDFA<T>::process_one_block(
    const IR::BasicBlock& block,
    std::unordered_set<const IR::BasicBlock*>& worklist,
    bool stop_on_unchanged) {
  worklist.erase(&block);

  if (!block.is_end()) {
    std::vector<const T*> children_data;
    for (auto child : block.children) {
      if (child != nullptr) {
        children_data.push_back(&storage_.get_data(child, Position::BEFORE));
      }
    }

    auto meet_result = meet(children_data);
    auto& after = storage_.get_data(&block, Position::AFTER);

    if (meet_result == after && stop_on_unchanged) {
      return;
    }

    after = meet_result;
  }

  bool was_changed = transfer_through_block(block, stop_on_unchanged);

  if (was_changed) {
    for (auto parent : block.parents) {
      worklist.insert(parent);
    }
  }
}

template <typename T>
void BackwardDFA<T>::start_dfa(const IR::Program& program) {
  storage_.clear();

  for (const auto& function : program.functions) {
    init(function);

    std::unordered_set<const IR::BasicBlock*> worklist;

    // TODO: choose better order
    // we must pass through each block at least once
    for (auto& block : function.basic_blocks) {
      worklist.insert(&block);
      process_one_block(block, worklist, false);
    }

    while (!worklist.empty()) {
      const IR::BasicBlock* current = *worklist.begin();

      process_one_block(*current, worklist, true);
    }
  }
}
}  // namespace Passes
