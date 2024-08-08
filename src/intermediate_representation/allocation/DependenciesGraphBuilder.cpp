#include "DependenciesGraphBuilder.h"

#include <fmt/core.h>

#include <iostream>

#include "intermediate_representation/Function.h"

void IR::DependenciesGraphBuilder::process_block(const Function& function,
                                                 const BasicBlock* block) {
  const auto& instructions = block->instructions;

  // transfer from parents only those temporaries that are used in this block
  // (or in blocks below)
  for (auto parent : block->parents) {
    auto live = storage_.get_live(parent, Position::AFTER);

    for (Value temp : live) {
      const auto& info = function.temporaries_info.find(temp)->second;
      if (info.using_blocks.contains(block)) {
        storage_.add_live(block, Position::BEFORE, temp);
      }
    }
  }

  // transfer to space before first variable
  storage_.add_live(block->instructions.front().get(), Position::BEFORE,
                    storage_.get_live(block, Position::BEFORE));

  // find last usage for each variable that do not escape
  std::unordered_map<Value, BaseInstruction*> last_usage;

  for (auto& instruction : instructions | std::views::reverse) {
    for (auto temporary :
         instruction->filter_arguments(ValueType::VIRTUAL_REGISTER)) {
      auto itr = function.temporaries_info.find(temporary);

      if (itr->second.is_used_in_descendants(block)) {
        continue;
      }

      last_usage.emplace(temporary, instruction.get());
    }
  }

  // special case: for function arguments that are not used in program we set
  // last usage to the first instruction in the block
  if (block == function.begin_block) {
    for (auto argument : function.arguments) {
      auto& info = function.temporaries_info.at(argument);
      if (!info.is_escaping() && !last_usage.contains(argument)) {
        last_usage[argument] = block->instructions.front().get();
      }
    }
  }

  // for each instruction propagate live
  for (auto itr = instructions.begin(); itr != instructions.end(); ++itr) {
    // transfer from previous instruction
    if (itr != instructions.begin()) {
      storage_.transfer_live(std::prev(itr)->get(), itr->get());
    }

    auto live_temporaries = storage_.get_live(itr->get(), Position::BEFORE);

    // remove dead temporaries
    std::erase_if(live_temporaries, [&last_usage, itr](Value temp) {
      auto last_usage_itr = last_usage.find(temp);

      return last_usage_itr != last_usage.end() &&
             last_usage_itr->second == itr->get();
    });

    // add return value to live temporaries
    if ((*itr)->has_return_value()) {
      live_temporaries.insert((*itr)->get_return_value());
    }

    // save result after instruction
    storage_.add_live(itr->get(), Position::AFTER, live_temporaries);
  }

  // tranfer result of last instruction into the end of the block
  storage_.add_live(
      block, Position::AFTER,
      storage_.get_live(block->instructions.back().get(), Position::AFTER));
}

void IR::DependenciesGraphBuilder::create_dependencies(
    const std::unordered_set<Value>& temporaries) {
  for (auto first = temporaries.begin(); first != temporaries.end(); ++first) {
    for (auto second = std::next(first); second != temporaries.end();
         ++second) {
      result_.add_dependency(*first, *second,
                             -TemporaryDependenciesGraph::kInfinity);
    }
  }
}

void IR::DependenciesGraphBuilder::print_result() const {
  for (auto& temp : result_.temporaries) {
    fmt::print("     {}", temp.temporary);
  }

  std::cout << std::endl;

  for (auto& row : result_.edges) {
    for (auto& info : row) {
      bool is_infinity =
          std::abs(info) == TemporaryDependenciesGraph::kInfinity;

      if (is_infinity) {
        bool is_negative = info < 0;
        std::cout << "    " << (is_negative ? "-inf" : "+inf");
      } else {
        fmt::print("{:7}", info);
      }
    }

    std::cout << std::endl;
  }
}

IR::TemporaryDependenciesGraph IR::DependenciesGraphBuilder::operator()(
    Function& function) {
  // std::cout << "Dependencies for " << function.name << std::endl;

  for (const auto& temp : function.temporaries_info | std::views::keys) {
    if (temp.value < function.arguments.size()) {
      result_.add_temporary(
          temp, 0,
          std::pair{temp.value, TemporaryDependenciesGraph::kInfinity});
    } else {
      result_.add_temporary(temp);
    }
  }

  for (auto argument : function.arguments) {
    storage_.add_live(function.begin_block, Position::BEFORE, argument);
  }

  // process each block and register live variables at each program point
  function.traverse_blocks([this, &function](const BasicBlock* block) {
    process_block(function, block);
  });

  // create edges between live variables
  // TODO: remove second action

  for (auto& block : function.basic_blocks) {
    for (auto& instruction : block.instructions) {
      auto& live_before =
          storage_.get_live(instruction.get(), Position::BEFORE);
      create_dependencies(live_before);

      auto& live_after = storage_.get_live(instruction.get(), Position::AFTER);
      create_dependencies(live_after);

      // std::cout << fmt::format("Before: {}\n", fmt::join(live_before, ", "));
      // std::cout << "Instruction: " << instruction->to_string() << std::endl;
      // std::cout << fmt::format("After: {}\n", fmt::join(live_after, ", "));
    }
  }

  // std::cout << std::endl;
  // print_result();

  return std::move(result_);
}
