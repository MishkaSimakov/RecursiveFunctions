#include "Function.h"

void IR::Function::replace_values(
    const std::unordered_map<Value, Value>& mapping) const {
  for (auto& block : basic_blocks) {
    for (auto& instruction : block.instructions) {
      instruction->replace_values(mapping);
    }
  }
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
