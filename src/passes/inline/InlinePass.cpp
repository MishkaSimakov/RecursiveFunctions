#include "InlinePass.h"

#include <algorithm>
#include <iostream>

#include "passes/PassManager.h"

void Passes::InlinePass::inline_function_call(
    IR::Function& function, IR::BasicBlock& block,
    IR::BasicBlock::InstructionsListT::iterator call_itr) {
  auto& call = static_cast<IR::FunctionCall&>(**call_itr);
  const auto& called_function = manager_.program.get_function(call.name);
  auto& blocks = function.basic_blocks;

  // split original basic block into two
  IR::BasicBlock& first_part = block;
  IR::BasicBlock& second_part = *function.add_block();

  for (auto move_itr = std::next(call_itr);
       move_itr != block.instructions.end();) {
    second_part.instructions.push_back(std::move(*move_itr));
    move_itr = first_part.instructions.erase(move_itr);
  }

  // insert basic blocks
  auto new_blocks_start = std::prev(blocks.end());
  std::unordered_map<const IR::BasicBlock*, IR::BasicBlock*> blocks_mapping;
  std::unordered_map<IR::Value, IR::Value> temporaries_mapping;

  size_t temporary_index = function.get_max_temporary_index();
  for (auto temp : called_function.temporaries_info | std::views::keys) {
    if (temp.value < called_function.arguments_count) {
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
    for (auto& child : current_block.children) {
      if (child != nullptr) {
        child = blocks_mapping[child];
      }
    }

    for (auto& parent : current_block.parents) {
      parent = blocks_mapping[parent];
    }

    for (auto& instruction : current_block.instructions) {
      auto* phi_node = dynamic_cast<IR::Phi*>(instruction.get());

      if (phi_node == nullptr) {
        continue;
      }

      for (auto& origin : phi_node->parents | std::views::keys) {
        origin = blocks_mapping[origin];
      }
    }
  }

  if (should_insert_phi) {
    auto phi = std::make_unique<IR::Phi>();
    phi->return_value = call.return_value;
    second_part.instructions.push_front(std::move(phi));
  }

  for (const auto end_block : called_function.end_blocks) {
    auto inserted_block = blocks_mapping[end_block];
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
  std::unordered_set<const IR::BasicBlock*> visited;
  join_blocks_recursive(function, function.begin_block, visited);
  function.finalize();
}

void Passes::InlinePass::join_blocks_recursive(
    IR::Function& function, IR::BasicBlock* block,
    std::unordered_set<const IR::BasicBlock*>& visited) {
  if (block == nullptr) {
    return;
  }

  if (block->is_end()) {
    return;
  }

  if (visited.contains(block)) {
    return;
  }

  auto& first_child = *block->children[0];
  if (block->children[1] == nullptr &&
      block->children[0]->parents.size() == 1) {
    for (auto& instruciton : first_child.instructions) {
      block->instructions.push_back(std::move(instruciton));
    }

    block->children = std::move(first_child.children);

    // preserve parents in correct state
    for (auto child : block->children) {
      if (child == nullptr) {
        continue;
      }

      for (auto& parent : child->parents) {
        if (parent == &first_child) {
          parent = block;
        }
      }
    }

    std::erase_if(
        function.basic_blocks,
        [&first_child](const IR::BasicBlock& b) { return &b == &first_child; });

    join_blocks_recursive(function, block, visited);
  }

  visited.insert(block);

  for (auto child : block->children) {
    join_blocks_recursive(function, child, visited);
  }
}

void Passes::InlinePass::apply() {
  for (auto& function : manager_.program.functions) {
    for (auto& block : function.basic_blocks) {
      for (auto itr = block.instructions.begin();
           itr != block.instructions.end(); ++itr) {
        auto* call_ptr = dynamic_cast<IR::FunctionCall*>(itr->get());
        if (call_ptr == nullptr) {
          continue;
        }

        if (manager_.program.get_function(call_ptr->name).is_recursive) {
          continue;
        }

        inline_function_call(function, block, itr);

        return;
      }
    }
  }
}
