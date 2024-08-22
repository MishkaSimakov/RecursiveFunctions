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
  std::unordered_map<std::pair<IR::Value, const IR::BasicBlock*>,
                     const IR::BasicBlock*>
      phi_values;

  void perform_analysis(const IR::Program&) override;

  std::unordered_map<IR::Value, TemporaryLivenessState> meet(
      std::span<IR::BasicBlock* const> children,
      const IR::BasicBlock& current) override;

  std::unordered_map<IR::Value, TemporaryLivenessState> transfer(
      const std::unordered_map<IR::Value, TemporaryLivenessState>& after,
      const IR::BaseInstruction& instruction,
      const IR::BasicBlock& current) override;

  void init(const IR::Function& function) override;
};
}  // namespace Passes
