#pragma once
#include "passes/analysis/Analyser.h"

namespace Passes {
class LoopSymbolicAnalysis : public Analyser {
 public:
  using Analyser::Analyser;

 protected:
  void perform_analysis(const IR::Program&) override;
};
}  // namespace Passes
