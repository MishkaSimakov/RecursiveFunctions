#include "SillyMoveErasurePass.h"

#include <iostream>

#include "passes/PassManager.h"

class EqualityStorage {
  std::unordered_map<IR::Value, IR::Value> equalities_;

 public:
  void clear() { equalities_.clear(); }

  void set_equal(IR::Value first, IR::Value second) {
    if (first == second) {
      return;
    }

    if (first > second) {
      std::swap(first, second);
    }

    equalities_[first] = second;
  }

  void remove(IR::Value value) {
    std::erase_if(equalities_,
                  [value](const std::pair<IR::Value, IR::Value>& pair) {
                    return pair.first == value || pair.second == value;
                  });
  }

  bool is_equal(IR::Value first, IR::Value second) const {
    if (first == second) {
      return true;
    }

    if (first > second) {
      std::swap(first, second);
    }

    auto itr = equalities_.find(first);
    return itr != equalities_.end() && itr->second == second;
  }
};

bool Passes::SillyMoveErasurePass::apply(IR::Function& function,
                                         IR::BasicBlock& block) {
  bool was_changed = false;

  EqualityStorage equalities;
  auto& instructions = block.instructions;

  for (auto itr = instructions.begin(); itr != instructions.end();) {
    auto& instr = **itr;

    if (instr.is_of_type<IR::FunctionCall>()) {
      equalities.clear();
      ++itr;
      continue;
    }

    if (instr.has_return_value()) {
      IR::Value return_value = instr.get_return_value();
      equalities.remove(return_value);
    }

    if (instr.is_of_type<IR::Move>()) {
      const auto& move = static_cast<const IR::Move&>(instr);

      if (equalities.is_equal(move.return_value, move.arguments[0])) {
        was_changed = true;

        itr = instructions.erase(itr);
        continue;
      }

      equalities.set_equal(move.return_value, move.arguments[0]);
    }

    ++itr;
  }

  return was_changed;
}
