#include "UnusedFunctionsEliminationPass.h"

#include <unordered_set>

#include "intermediate_representation/Function.h"
#include "passes/PassManager.h"
#include "utils/Constants.h"

bool Passes::UnusedFunctionsEliminationPass::apply(IR::Program& program) {
  auto& entrypoint = *program.get_function(Constants::entrypoint);
  used_.insert(Constants::entrypoint);

  find_used_recursively(program, entrypoint);

  size_t erased_count =
      std::erase_if(program.functions, [this](const IR::Function& function) {
        return !used_.contains(function.name);
      });

  return erased_count != 0;
}

void Passes::UnusedFunctionsEliminationPass::find_used_recursively(
    IR::Program& program, const IR::Function& current) {
  for (auto& block : current.basic_blocks) {
    for (auto& instruction : block.instructions) {
      auto* call_ptr = dynamic_cast<const IR::FunctionCall*>(instruction.get());

      if (call_ptr == nullptr) {
        continue;
      }

      auto called_function = program.get_function(call_ptr->name);
      auto [itr, was_inserted] = used_.insert(call_ptr->name);

      if (was_inserted && called_function != nullptr) {
        find_used_recursively(program, *called_function);
      }
    }
  }
}
