#include "LivenessAnalysis.h"

void Passes::LivenessAnalysis::perform_analysis(const IR::Program& program) {
  for (auto& function : program.functions) {
    before_function(function);

    function.postorder_traversal([this, &function](const IR::BasicBlock* block) {
      process_block(function, *block);
    });
  }
}

void Passes::LivenessAnalysis::before_function(const IR::Function& function) {
  for (auto argument : function.arguments) {
    live_storage.add_live(function.begin_block, Position::BEFORE, argument);
  }
}

void Passes::LivenessAnalysis::process_block(const IR::Function& function,
                                             const IR::BasicBlock& block) {
  const auto& instructions = block.instructions;

  // transfer from parents only those temporaries that are used in this block
  // (or in blocks below)
  for (auto parent : block.parents) {
    auto live = live_storage.get_live(parent, Position::AFTER);

    for (IR::Value temp : live) {
      const auto& info = function.temporaries_info.find(temp)->second;
      if (info.using_blocks.contains(&block)) {
        live_storage.add_live(&block, Position::BEFORE, temp);
      }
    }
  }

  // find last usage for each variable that do not escape
  std::unordered_map<IR::Value, IR::BaseInstruction*> last_usage;

  for (auto& instruction : instructions | std::views::reverse) {
    for (auto temporary :
         instruction->filter_arguments(IR::ValueType::VIRTUAL_REGISTER)) {
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
    for (auto& argument : function.arguments) {
      auto& info = function.temporaries_info.at(argument);
      if (!info.is_escaping() && !last_usage.contains(argument)) {
        last_usage[argument] = block.instructions.front().get();
      }
    }
  }

  // for each instruction propagate live
  for (auto itr = instructions.begin(); itr != instructions.end(); ++itr) {
    // transfer from previous instruction
    if (itr != instructions.begin()) {
      live_storage.transfer_live(std::prev(itr)->get(), itr->get());
    }

    auto live_temporaries = live_storage.get_live(itr->get(), Position::BEFORE);

    // remove dead temporaries
    std::erase_if(live_temporaries, [&last_usage, itr](IR::Value temp) {
      auto last_usage_itr = last_usage.find(temp);

      return last_usage_itr != last_usage.end() &&
             last_usage_itr->second == itr->get();
    });

    // add return value to live temporaries
    if ((*itr)->has_return_value()) {
      live_temporaries.insert((*itr)->get_return_value());
    }

    // save result after instruction
    live_storage.add_live(itr->get(), Position::AFTER, live_temporaries);
  }
}
