#include "LivenessAnalysis.h"

#include <iostream>

bool Passes::LivenessAnalysis::is_escaping_block(const IR::BasicBlock* block,
                                                 IR::Value value) const {
  for (auto child : block->children) {
    if (child != nullptr && should_transfer_value(value, child)) {
      return true;
    }
  }

  return false;
}

void Passes::LivenessAnalysis::perform_analysis(const IR::Program& program) {
  live_storage.clear();
  blocks_info.clear();

  for (auto& function : program.functions) {
    std::vector<const IR::BasicBlock*> order;
    order.reserve(function.basic_blocks.size());

    function.reversed_postorder_traversal(
        [&order](const IR::BasicBlock* block) { order.push_back(block); });

    // arguments assignment happens on top of the entry block
    auto& entryblock_info = blocks_info[function.begin_block];
    for (auto argument : function.arguments) {
      entryblock_info.used[argument] = BlockInfo::UsageType::ASSIGNED;
    }

    for (auto block : order) {
      auto& info = blocks_info[block];

      for (auto& instruction : block->instructions) {
        auto used =
            instruction->filter_arguments(IR::ValueType::VIRTUAL_REGISTER);

        for (IR::Value value : used) {
          auto [itr, was_inserted] =
              info.used.emplace(value, BlockInfo::UsageType::USED);
          auto& current_state = itr->second;

          if (current_state == BlockInfo::UsageType::ASSIGNED) {
            current_state = BlockInfo::UsageType::ASSIGNED_THEN_USED;
          }
        }

        if (instruction->has_return_value()) {
          auto return_value = instruction->get_return_value();
          auto [itr, was_inserted] =
              info.used.emplace(return_value, BlockInfo::UsageType::ASSIGNED);
          auto& current_state = itr->second;

          if (current_state == BlockInfo::UsageType::USED) {
            current_state = BlockInfo::UsageType::USED_THEN_ASSIGNED;
          }
        }
      }
    }

    std::unordered_set<const IR::BasicBlock*> visited;
    for (auto& block : function.basic_blocks) {
      auto& info = blocks_info.at(&block);

      for (auto [value, usage_type] : info.used) {
        // if block contains value assignment we start propagation from top to
        // bottom and write this value into reachable_from_top
        if (usage_type == BlockInfo::UsageType::ASSIGNED ||
            usage_type == BlockInfo::UsageType::ASSIGNED_THEN_USED ||
            usage_type == BlockInfo::UsageType::USED_THEN_ASSIGNED) {
          visited.clear();
          propagate_temporary_from_top(value, &block, visited);
        }

        // if block contains value usage we start propagation from bottom to top
        // and write this value into reachable_from_bottom
        if (usage_type == BlockInfo::UsageType::USED ||
            usage_type == BlockInfo::UsageType::ASSIGNED_THEN_USED ||
            usage_type == BlockInfo::UsageType::USED_THEN_ASSIGNED) {
          visited.clear();
          propagate_temporary_from_bottom(value, &block, visited);
        }
      }
    }

    // alive is instersection between reachable from top and bottom
    for (auto block : order) {
      auto& info = blocks_info[block];

      // set intersection
      for (auto value : info.reachable_from_top) {
        if (info.reachable_from_bottom.contains(value)) {
          info.alive.insert(value);
        }
      }
    }

    // then calculate precise instructions where they are alive
    before_function(function);

    function.reversed_postorder_traversal(
        [this, &function](const IR::BasicBlock* block) {
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
  const auto& info = blocks_info.at(&block);
  const auto& instructions = block.instructions;

  if (instructions.empty()) {
    throw std::runtime_error(
        "Empty blocks are not supported by LivenessAnalysisPass.");
  }

  // transfer from parents only those temporaries that are used in this block
  // (or in blocks below)
  for (auto parent : block.parents) {
    auto live = live_storage.get_live(parent, Position::AFTER);

    for (IR::Value temp : live) {
      if (should_transfer_value(temp, &block)) {
        live_storage.add_live(&block, Position::BEFORE, temp);
      }
    }
  }

  // find last usage for each variable that do not escape
  std::unordered_map<IR::Value, IR::BaseInstruction*> last_usage;

  for (auto& instruction : instructions | std::views::reverse) {
    for (auto value :
         instruction->filter_arguments(IR::ValueType::VIRTUAL_REGISTER)) {
      if (!is_escaping_block(&block, value)) {
        last_usage.emplace(value, instruction.get());
      }
    }
  }

  // special case: for function arguments that are not used in program we set
  // last usage to the first instruction in the block
  if (&block == function.begin_block) {
    for (auto argument : function.arguments) {
      if (!is_escaping_block(&block, argument)) {
        last_usage.emplace(argument, block.instructions.front().get());
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

void Passes::LivenessAnalysis::propagate_temporary_from_top(
    IR::Value value, const IR::BasicBlock* current,
    std::unordered_set<const IR::BasicBlock*>& visited) {
  if (visited.contains(current)) {
    return;
  }

  visited.insert(current);
  blocks_info.at(current).reachable_from_top.insert(value);

  for (auto next : current->children) {
    if (next == nullptr) {
      continue;
    }

    auto usage_type = blocks_info[next].used[value];

    // if assignment comes earlier than usage in this block we stop propagation
    if (usage_type == BlockInfo::UsageType::ASSIGNED ||
        usage_type == BlockInfo::UsageType::ASSIGNED_THEN_USED) {
      continue;
    }

    propagate_temporary_from_top(value, next, visited);
  }
}

void Passes::LivenessAnalysis::propagate_temporary_from_bottom(
    IR::Value value, const IR::BasicBlock* current,
    std::unordered_set<const IR::BasicBlock*>& visited) {
  if (visited.contains(current)) {
    return;
  }

  visited.insert(current);
  blocks_info.at(current).reachable_from_bottom.insert(value);

  if (blocks_info.at(current).used[value] ==
      BlockInfo::UsageType::ASSIGNED_THEN_USED) {
    return;
  }

  for (auto next : current->parents) {
    if (next == nullptr) {
      continue;
    }

    auto usage_type = blocks_info.at(next).used[value];

    // if assignment comes earlier than usage in this block we stop propagation
    if (usage_type == BlockInfo::UsageType::ASSIGNED ||
        usage_type == BlockInfo::UsageType::USED_THEN_ASSIGNED) {
      continue;
    }

    propagate_temporary_from_bottom(value, next, visited);
  }
}

bool Passes::LivenessAnalysis::should_transfer_value(
    IR::Value value, const IR::BasicBlock* to) const {
  auto to_info = blocks_info.at(to);
  if (!to_info.alive.contains(value)) {
    return false;
  }

  auto usage_type = to_info.used.at(value);
  if (usage_type == BlockInfo::UsageType::ASSIGNED ||
      usage_type == BlockInfo::UsageType::ASSIGNED_THEN_USED) {
    return false;
  }

  return true;
}
