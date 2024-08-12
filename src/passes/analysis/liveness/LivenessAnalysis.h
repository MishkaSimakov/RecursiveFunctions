#pragma once
#include "LiveTemporariesStorage.h"
#include "passes/analysis/Analyser.h"

namespace Passes {
class LivenessAnalysis final : public Analyser {
 public:
  using Analyser::Analyser;

  const LiveTemporariesStorage& get_liveness_info() const {
    return live_storage;
  }

 protected:
  LiveTemporariesStorage live_storage;

  using Position = LiveTemporariesStorage::Position;

  void perform_analysis(const IR::Program&) override;

  void before_function(const IR::Function&);

  void process_block(const IR::Function&, const IR::BasicBlock&);
};
}  // namespace Passes
