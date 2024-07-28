#include "RandomOrderPass.h"

#include "intermediate_representation/Function.h"
#include "passes/PassManager.h"

void Passes::RandomOrderPass::apply() {
  auto& functions = manager_.program.functions;

  for (auto& function : functions) {
    before_function(function);

    for (auto& block : function.basic_blocks) {
      process_block(function, block);
    }

    after_function(function);
  }
}
