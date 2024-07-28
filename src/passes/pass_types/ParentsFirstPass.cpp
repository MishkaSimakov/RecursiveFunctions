#include "ParentsFirstPass.h"

#include "intermediate_representation/Function.h"
#include "passes/PassManager.h"

void Passes::ParentsFirstPass::apply() {
  auto& functions = manager_.program.functions;

  for (auto& function : functions) {
    before_function(function);

    function.traverse_blocks([this, &function](IR::BasicBlock* block) {
      process_block(function, *block);
    });

    after_function(function);
  }
}
