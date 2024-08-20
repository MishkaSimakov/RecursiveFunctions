#pragma once
#include "LiveTemporariesStorage.h"
#include "passes/analysis/Analyser.h"
#include "passes/analysis/dfa/BackwardDFA.h"

namespace Passes {
enum class TemporaryLivenessState {
  TOP = 0,
  DEAD = 1,
  LIVE = 2,
};

class LivenessAnalysis final
    : public Analyser,
      public BackwardDFA<
          std::unordered_map<IR::Value, TemporaryLivenessState>> {
 public:
  using Analyser::Analyser;

  const auto& get_liveness_info() const { return storage_; }

 protected:
  void perform_analysis(const IR::Program&) override;

  std::unordered_map<IR::Value, TemporaryLivenessState> meet(
      std::vector<const std::unordered_map<IR::Value, TemporaryLivenessState>*>
          children) const override;

  std::unordered_map<IR::Value, TemporaryLivenessState> transfer(
      const std::unordered_map<IR::Value, TemporaryLivenessState>& after,
      const IR::BaseInstruction& instruction) const override;

  void init(const IR::Function& function) override;
};
}  // namespace Passes
