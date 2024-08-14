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

 // protected:
  LiveTemporariesStorage live_storage;

  struct BlockInfo {
    enum class UsageType {
      NONE,
      ASSIGNED,
      USED,
      USED_THEN_ASSIGNED,
      ASSIGNED_THEN_USED
    };

    std::unordered_set<IR::Value> alive;
    std::unordered_set<IR::Value> reachable_from_top;
    std::unordered_set<IR::Value> reachable_from_bottom;
    std::unordered_map<IR::Value, UsageType> used;
  };

  std::unordered_map<const IR::BasicBlock*, BlockInfo> blocks_info;

  using Position = LiveTemporariesStorage::Position;

  bool is_escaping_block(const IR::BasicBlock*, IR::Value) const;

  void perform_analysis(const IR::Program&) override;

  void before_function(const IR::Function&);

  void process_block(const IR::Function&, const IR::BasicBlock&);

  void propagate_temporary_from_top(IR::Value, const IR::BasicBlock*,
                                    std::unordered_set<const IR::BasicBlock*>&);

  void propagate_temporary_from_bottom(
      IR::Value, const IR::BasicBlock*,
      std::unordered_set<const IR::BasicBlock*>&);

  bool should_transfer_value(IR::Value, const IR::BasicBlock*) const;
};
}  // namespace Passes
