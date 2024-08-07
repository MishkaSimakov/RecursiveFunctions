#pragma once

#include <functional>
#include <ranges>
#include <stack>
#include <unordered_map>
#include <unordered_set>

#include "TemporaryDependenciesGraph.h"
#include "intermediate_representation/BasicBlock.h"
#include "intermediate_representation/Instruction.h"
#include "intermediate_representation/Value.h"

namespace IR {
class DependenciesGraphBuilder {
  struct LiveTemporariesStorage {
    enum class Position { BEFORE, AFTER };

   private:
    // live temporaries in points before/after each instruction
    mutable std::unordered_map<std::pair<const BaseInstruction*, Position>,
                               std::unordered_set<Value>>
        instructions_live_temporaries;

    // live temporaries in points before/after each basic block
    mutable std::unordered_map<std::pair<const BasicBlock*, Position>,
                               std::unordered_set<Value>>
        blocks_live_temporaries;

   public:
    const std::unordered_set<Value>& get_live(
        const BaseInstruction* instruction, Position position) const {
      return instructions_live_temporaries[{instruction, position}];
    }

    const std::unordered_set<Value>& get_live(const BasicBlock* block,
                                              Position position) const {
      return blocks_live_temporaries[{block, position}];
    }

    void add_live(const BaseInstruction* instr, Position position, Value temp) {
      instructions_live_temporaries[{instr, position}].insert(temp);
    }

    void add_live(const BaseInstruction* instr, Position position,
                  const std::ranges::range auto& range) {
      std::ranges::for_each(range, [this, instr, position](Value temp) {
        add_live(instr, position, temp);
      });
    }

    void add_live(const BasicBlock* block, Position position, Value temp) {
      blocks_live_temporaries[{block, position}].insert(temp);
    }

    void add_live(const BasicBlock* block, Position position,
                  const std::ranges::range auto& range) {
      std::ranges::for_each(range, [this, block, position](Value temp) {
        add_live(block, position, temp);
      });
    }

    void transfer_live(const BaseInstruction* from, const BaseInstruction* to) {
      instructions_live_temporaries[{to, Position::BEFORE}] =
          instructions_live_temporaries[{from, Position::AFTER}];
    }
  };

  using Position = LiveTemporariesStorage::Position;

  LiveTemporariesStorage storage_;
  TemporaryDependenciesGraph result_;

  void process_block(const Function& function, const BasicBlock*);
  void create_dependencies(const std::unordered_set<Value>&);
  void print_result() const;

 public:
  DependenciesGraphBuilder() = default;

  TemporaryDependenciesGraph operator()(Function&);
};
}  // namespace IR
