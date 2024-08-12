#pragma once
#include <bitset>

#include "passes/analysis/Analyser.h"

namespace Passes {
class DominatorsAnalysis final : public Analyser {
 public:
  using Analyser::Analyser;

  bool dominate(size_t, const IR::BasicBlock*, const IR::BasicBlock*) const;

 protected:
  BasicBlocksData<std::unordered_set<const IR::BasicBlock*>> dominators;

  void perform_analysis(const IR::Program&) override;

  void process_function(const IR::Function&);
};
}  // namespace Passes
