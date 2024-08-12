#include "UnusedFunctionsEliminationPass.h"

#include <unordered_set>

#include "intermediate_representation/Function.h"
#include "passes/PassManager.h"

void Passes::UnusedFunctionsEliminationPass::apply() {
  auto& entrypoint = manager_.program.get_function(IR::Function::entrypoint);
  used_.insert(IR::Function::entrypoint);

  find_used_recursively(entrypoint);

  std::erase_if(manager_.program.functions,
                [this](const IR::Function& function) {
                  return !used_.contains(function.name);
                });
}

void Passes::UnusedFunctionsEliminationPass::find_used_recursively(
    const IR::Function& current) {
  for (auto& block : current.basic_blocks) {
    for (auto& instruction : block.instructions) {
      auto* call_ptr = dynamic_cast<const IR::FunctionCall*>(instruction.get());

      if (call_ptr == nullptr) {
        continue;
      }

      auto& called_function = manager_.program.get_function(call_ptr->name);
      auto [itr, was_inserted] = used_.insert(call_ptr->name);

      if (was_inserted) {
        find_used_recursively(called_function);
      }
    }
  }
}
