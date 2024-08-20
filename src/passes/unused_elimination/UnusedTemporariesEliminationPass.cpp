#include "UnusedTemporariesEliminationPass.h"

#include "passes/PassManager.h"

bool Passes::UnusedTemporariesEliminationPass::apply(IR::Function& function) {
  auto used = find_used_in_function(function);
  bool was_changed = false;

  for (auto& bb : function.basic_blocks) {
    size_t count = std::erase_if(
        bb.instructions,
        [&used](const std::unique_ptr<IR::BaseInstruction>& instruction) {
          return instruction->has_return_value() &&
                 !used.contains(instruction->get_return_value());
        });

    if (count != 0) {
      was_changed = true;
    }
  }

  return was_changed;
}

std::unordered_set<IR::Value>
Passes::UnusedTemporariesEliminationPass::find_used_in_function(
    const IR::Function& function) const {
  size_t size = function.get_max_temporary_index() + 1;

  std::vector dependencies(size, std::vector<IR::Value>{});
  std::vector<IR::Value> used;

  for (auto& bb : function.basic_blocks) {
    for (auto& instruction : bb.instructions) {
      if (instruction->is_control_flow_instruction()) {
        auto cf_temps =
            instruction->filter_arguments(IR::ValueType::VIRTUAL_REGISTER);

        used.insert(used.end(), cf_temps.begin(), cf_temps.end());
        continue;
      }

      if (instruction->has_return_value()) {
        auto return_value = instruction->get_return_value();
        auto arguments =
            instruction->filter_arguments(IR::ValueType::VIRTUAL_REGISTER);

        auto& deps = dependencies[return_value.value];
        deps.insert(deps.end(), arguments.begin(), arguments.end());
      }
    }
  }

  std::unordered_set<IR::Value> result;
  for (auto used_value : used) {
    mark_used_recursively(used_value, dependencies, result);
  }

  return result;
}

void Passes::UnusedTemporariesEliminationPass::mark_used_recursively(
    IR::Value current, const std::vector<std::vector<IR::Value>>& dependencies,
    std::unordered_set<IR::Value>& used) const {
  auto [itr, was_inserted] = used.insert(current);

  if (!was_inserted) {
    return;
  }

  for (auto value : dependencies[current.value]) {
    mark_used_recursively(value, dependencies, used);
  }
}
