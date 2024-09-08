#include "InlinePass.h"

#include <algorithm>
#include <iostream>

#include "passes/PassManager.h"
#include "passes/analysis/dominators/DominatorsAnalysis.h"

void Passes::InlinePass::inline_function_call(
    IR::Program& program, IR::Function& function, IR::BasicBlock& block,
    IR::BasicBlock::InstructionsListT::iterator call_itr) {
  auto& call = static_cast<IR::FunctionCall&>(**call_itr);
  const auto& called_function = program.get_function(call.name);
  auto& blocks = function.basic_blocks;

  // split original basic block into two
  IR::BasicBlock& first_part = block;
  IR::BasicBlock& second_part = function.split_block(block, call_itr);

  // insert basic blocks
  auto new_blocks_start = std::prev(blocks.end());
  std::unordered_map<const IR::BasicBlock*, IR::BasicBlock*> blocks_mapping;
  std::unordered_map<IR::Value, IR::Value> temporaries_mapping;

  size_t temporary_index = function.get_max_temporary_index();
  for (auto temp : called_function.temporaries) {
    if (temp.value < called_function.arguments.size()) {
      // treat as argument
      temporaries_mapping[temp] = call.arguments[temp.value];
    } else {
      temporaries_mapping[temp] =
          IR::Value(++temporary_index, IR::ValueType::VIRTUAL_REGISTER);
    }
  }

  for (auto& basic_block : called_function.basic_blocks) {
    auto copy = basic_block.copy_instructions();
    copy.children = basic_block.children;

    // we should update temporaries
    for (auto& instruction : copy.instructions) {
      instruction->replace_values(temporaries_mapping);
    }

    blocks.emplace_back(std::move(copy));

    blocks_mapping[&basic_block] = &blocks.back();
  }

  // we should update children, parents and phi node pointers
  bool should_insert_phi = called_function.end_blocks.size() > 1;

  for (auto itr = std::next(new_blocks_start); itr != blocks.end(); ++itr) {
    auto& current_block = *itr;

    for (auto& child : current_block.nonnull_children()) {
      child = blocks_mapping.at(child);
    }

    for (auto& parent : current_block.parents) {
      parent = blocks_mapping.at(parent);
    }

    function.replace_phi_parents(blocks_mapping);
  }

  if (should_insert_phi) {
    auto phi = std::make_unique<IR::Phi>();
    phi->return_value = call.return_value;
    second_part.instructions.push_front(std::move(phi));
  }

  for (const auto end_block : called_function.end_blocks) {
    auto inserted_block = blocks_mapping.at(end_block);
    auto return_value =
        static_cast<IR::Return*>(inserted_block->instructions.back().get())
            ->arguments[0];

    inserted_block->instructions.pop_back();

    if (should_insert_phi) {
      static_cast<IR::Phi*>(second_part.instructions.front().get())
          ->parents.emplace_back(inserted_block, return_value);
    } else {
      inserted_block->instructions.push_back(
          std::make_unique<IR::Move>(call.return_value, return_value));
    }
  }

  // connect inserted blocks
  first_part.children = {blocks_mapping[called_function.begin_block]};
  for (auto end_block : called_function.end_blocks) {
    blocks_mapping[end_block]->children = {&second_part};
  }

  // remove original function call
  block.instructions.erase(call_itr);

  function.finalize();
  function.simplify_blocks();
}

bool Passes::InlinePass::apply(IR::Program& program) {
  bool was_changed = false;
  bool was_changed_on_current_iteration;

  do {
    was_changed_on_current_iteration = false;

    for (auto& function : program.functions) {
      bool was_function_changed = false;

      for (auto& block : function.basic_blocks) {
        size_t inline_threshold = 10;

        for (auto itr = block.instructions.begin();
             itr != block.instructions.end(); ++itr) {
          auto* call_ptr = dynamic_cast<IR::FunctionCall*>(itr->get());
          if (call_ptr == nullptr) {
            continue;
          }

          if (program.get_function(call_ptr->name).is_recursive()) {
            continue;
          }

          if (program.get_function(call_ptr->name).size() <= inline_threshold) {
            inline_function_call(program, function, block, itr);
            was_changed_on_current_iteration = true;
            was_function_changed = true;
            was_changed = true;

            break;
          }
        }

        if (was_function_changed) {
          break;
        }
      }
    }

    if (was_changed_on_current_iteration) {
      manager_.invalidate();
    }
  } while (was_changed_on_current_iteration);

  return was_changed;
}
