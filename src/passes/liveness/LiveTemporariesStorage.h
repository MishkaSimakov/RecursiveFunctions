#pragma once

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "intermediate_representation/Instruction.h"
#include "intermediate_representation/BasicBlock.h"

namespace Passes {
struct LiveTemporariesStorage {
  enum class Position { BEFORE, AFTER };

 private:
  // live temporaries in points before/after each instruction
  mutable std::unordered_map<std::pair<const IR::Instruction*, Position>,
                             std::unordered_set<IR::Temporary>>
      instructions_live_temporaries;

 public:
  const std::unordered_set<IR::Temporary>& get_live(
      const IR::Instruction* instruction, Position position) const {
    return instructions_live_temporaries[{instruction, position}];
  }

  const std::unordered_set<IR::Temporary>& get_live(const IR::BasicBlock* block,
                                                    Position position) const {
    if (position == Position::BEFORE) {
      return instructions_live_temporaries[{block->instructions.front().get(),
                                            Position::BEFORE}];
    }

    return instructions_live_temporaries[{block->instructions.back().get(),
                                          Position::AFTER}];
  }

  void add_live(const IR::Instruction* instr, Position position,
                IR::Temporary temp) {
    instructions_live_temporaries[{instr, position}].insert(temp);
  }

  void add_live(const IR::BasicBlock* block, Position position,
                IR::Temporary temp) {
    if (position == Position::BEFORE) {
      instructions_live_temporaries[{block->instructions.front().get(),
                                     Position::BEFORE}]
          .insert(temp);
    } else {
      instructions_live_temporaries[{block->instructions.back().get(),
                                     Position::AFTER}]
          .insert(temp);
    }
  }

  void add_live(const IR::Instruction* instr, Position position,
                const std::ranges::range auto& range) {
    std::ranges::for_each(range, [this, instr, position](IR::Temporary temp) {
      add_live(instr, position, temp);
    });
  }

  void add_live(const IR::BasicBlock* block, Position position,
                const std::ranges::range auto& range) {
    std::ranges::for_each(range, [this, block, position](IR::Temporary temp) {
      add_live(block, position, temp);
    });
  }

  void transfer_live(const IR::Instruction* from, const IR::Instruction* to) {
    instructions_live_temporaries[{to, Position::BEFORE}] =
        instructions_live_temporaries[{from, Position::AFTER}];
  }
};
}  // namespace Passes
