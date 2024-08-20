#pragma once
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "passes/analysis/liveness/LivenessAnalysis.h"
#include "passes/pass_types/BasicBlockLevelPass.h"

namespace Passes {
struct PrintPassConfig {
  bool print_live_info = false;
  bool print_blocks_addresses = false;
};

class PrintPass : public BasicBlockLevelPass<ReversedPostBasicBlocksOrder> {
  std::ostream& os_;
  mutable std::unordered_map<const IR::BasicBlock*, size_t> indices_;
  PrintPassConfig config_;

  std::string get_block_name(const IR::BasicBlock&) const;

  void print_live_info(
      const std::unordered_map<IR::Value, TemporaryLivenessState>&);

 protected:
  bool apply(IR::Function& function, IR::BasicBlock& block) override;
  void before_function(const IR::Function& function) const override;
  void after_function(const IR::Function& function) const override;

 public:
  PrintPass(PassManager&, std::ostream&, const PrintPassConfig& = {});
};
}  // namespace Passes
