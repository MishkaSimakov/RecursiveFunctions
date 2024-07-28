#include "LivenessPass.h"

#include "Constants.h"
#include "intermediate_representation/Function.h"
#include "passes/PassManager.h"

void Passes::LivenessPass::before_function(IR::Function& function) {
  for (size_t i = 0; i < function.arguments_count; ++i) {
    storage().add_live(function.begin_block, Position::BEFORE,
                       IR::Temporary{i});
  }
}

void Passes::LivenessPass::process_block(IR::Function& function,
                                         IR::BasicBlock& block) {
  const auto& instructions = block.instructions;

  // transfer from parents only those temporaries that are used in this block
  // (or in blocks below)
  for (auto parent : block.parents) {
    auto live = storage().get_live(parent, Position::AFTER);

    for (IR::Temporary temp : live) {
      const auto& info = function.temporaries_info.find(temp)->second;
      if (info.using_blocks.contains(&block)) {
        storage().add_live(&block, Position::BEFORE, temp);
      }
    }
  }

  // find last usage for each variable that do not escape
  std::unordered_map<IR::Temporary, IR::Instruction*> last_usage;

  for (auto& instruction : instructions | std::views::reverse) {
    for (auto temporary : instruction->get_temporaries_in_arguments()) {
      auto itr = function.temporaries_info.find(temporary);

      if (itr->second.is_used_in_descendants(&block)) {
        continue;
      }

      last_usage.emplace(temporary, instruction.get());
    }
  }

  // special case: for function arguments that are not used in program we set
  // last usage to the first instruction in the block
  if (&block == function.begin_block) {
    for (size_t i = 0; i < function.arguments_count; ++i) {
      IR::Temporary temporary{i};

      auto& info = function.temporaries_info.find(temporary)->second;
      if (!info.is_escaping() && !last_usage.contains(temporary)) {
        last_usage[temporary] = block.instructions.front().get();
      }
    }
  }

  // for each instruction propagate live
  for (auto itr = instructions.begin(); itr != instructions.end(); ++itr) {
    // transfer from previous instruction
    if (itr != instructions.begin()) {
      storage().transfer_live(std::prev(itr)->get(), itr->get());
    }

    auto live_temporaries = storage().get_live(itr->get(), Position::BEFORE);

    // remove dead temporaries
    std::erase_if(live_temporaries, [&last_usage, itr](IR::Temporary temp) {
      auto last_usage_itr = last_usage.find(temp);

      return last_usage_itr != last_usage.end() &&
             last_usage_itr->second == itr->get();
    });

    // add return value to live temporaries
    if ((*itr)->has_return_value()) {
      live_temporaries.insert((*itr)->result_destination);
    }

    // save result after instruction
    storage().add_live(itr->get(), Position::AFTER, live_temporaries);
  }
}
