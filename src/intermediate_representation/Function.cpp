#include "Function.h"

void IR::Function::simplify_blocks_recursive(
    BasicBlock* block, std::unordered_set<const BasicBlock*>& visited) {
  if (block == nullptr) {
    return;
  }

  if (block->is_end()) {
    visited.insert(block);
    return;
  }

  if (visited.contains(block)) {
    return;
  }

  auto& first_child = *block->children[0];
  if (block->has_one_child() && first_child.has_one_parent()) {
    // remove jump instruction
    block->instructions.pop_back();

    for (auto& instruction : first_child.instructions) {
      // TODO: this line filtering out phi instructions with only one parent.
      // Move this part into some other pass
      if (instruction->is_of_type<Phi>()) {
        auto& phi = static_cast<const Phi&>(*instruction);
        block->instructions.push_back(
            std::make_unique<Move>(phi.return_value, phi.parents[0].second));
      } else {
        block->instructions.push_back(std::move(instruction));
      }
    }

    block->children = std::move(first_child.children);

    // preserve parents in correct state
    for (auto child : first_child.children) {
      if (child == nullptr) {
        continue;
      }

      for (auto& parent : child->parents) {
        if (parent == &first_child) {
          parent = block;
        }
      }
    }

    // preserve phi nodes in correct state
    std::unordered_map<const BasicBlock*, BasicBlock*> mapping;
    mapping.emplace(&first_child, block);
    replace_phi_parents(mapping);

    std::erase_if(basic_blocks, [&first_child](const BasicBlock& b) {
      return &b == &first_child;
    });

    simplify_blocks_recursive(block, visited);
  }

  visited.insert(block);

  for (auto child : block->children) {
    simplify_blocks_recursive(child, visited);
  }
}

void IR::Function::replace_values(
    const std::unordered_map<Value, Value>& mapping) const {
  for (auto& block : basic_blocks) {
    for (auto& instruction : block.instructions) {
      instruction->replace_values(mapping);
    }
  }
}

IR::Value IR::Function::allocate_vreg() {
  size_t index = get_max_temporary_index() + 1;
  auto temp = Value(index, ValueType::VIRTUAL_REGISTER);
  temporaries.insert(temp);

  return temp;
}

void IR::Function::finalize() {
  // clear old information
  temporaries.clear();
  end_blocks.clear();
  calls.clear();

  for (auto& block : basic_blocks) {
    block.parents.clear();
  }

  // add arguments to temporaries info
  for (auto argument : arguments) {
    temporaries.insert(argument);
  }

  for (auto& block : basic_blocks) {
    for (auto child : block.children) {
      if (child != nullptr) {
        child->parents.push_back(&block);
      }
    }

    // fill end blocks data
    if (block.is_end()) {
      end_blocks.push_back(&block);
    }

    // add jumps
    if (block.has_one_child() &&
        (block.instructions.empty() ||
         !block.instructions.back()->is_of_type<Jump>())) {
      block.instructions.push_back(std::make_unique<Jump>());
    }

    for (const auto& instruction : block.instructions) {
      // find all used temporaries
      auto used = instruction->filter_arguments(ValueType::VIRTUAL_REGISTER);
      temporaries.insert(used.begin(), used.end());

      if (instruction->has_return_value()) {
        temporaries.insert(instruction->get_return_value());
      }

      // gather info about calls
      if (auto* call = dynamic_cast<const FunctionCall*>(instruction.get());
          call != nullptr) {
        calls.insert(call->name);
      }
    }
  }
}

void IR::Function::simplify_blocks() {
  std::unordered_set<const BasicBlock*> visited;

  simplify_blocks_recursive(begin_block, visited);

  std::erase_if(basic_blocks, [&visited](const BasicBlock& block) {
    return !visited.contains(&block);
  });

  finalize();
}

void IR::Function::replace_phi_parents(
    const std::unordered_map<const BasicBlock*, BasicBlock*>& mapping) {
  for (auto& block : basic_blocks) {
    for (auto& instruction : block.instructions) {
      auto* phi_node = dynamic_cast<IR::Phi*>(instruction.get());

      if (phi_node == nullptr) {
        continue;
      }

      for (auto& origin : phi_node->parents | std::views::keys) {
        auto itr = mapping.find(origin);

        if (itr != mapping.end()) {
          origin = itr->second;
        }
      }
    }
  }
}
