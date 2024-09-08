#include "LoopShrinkPass.h"

#include "passes/analysis/symbolic_analysis/LoopSymbolicAnalysis.h"

bool Passes::LoopShrinkPass::apply(IR::Function& function) {
  auto analysis = manager_.get_analysis<LoopSymbolicAnalysis>();
  return false;
}
