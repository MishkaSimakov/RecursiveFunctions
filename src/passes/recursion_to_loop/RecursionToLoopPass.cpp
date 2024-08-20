#include "RecursionToLoopPass.h"

#include "passes/PassManager.h"

void Passes::RecursionToLoopPass::apply() {
  for (auto& function : manager_.program.functions) {
    if (!function.is_recursive()) {
      continue;
    }


  }
}
