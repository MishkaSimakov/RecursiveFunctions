#pragma once
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "passes/pass_types/ParentsFirstPass.h"

namespace Passes {
struct PrintPassConfig {
  bool print_live_info = false;
  bool print_blocks_addresses = false;
};

class PrintPass : public ParentsFirstPass {
  std::ostream& os_;
  std::unordered_map<const IR::BasicBlock*, size_t> indices_;
  PrintPassConfig config_;

  std::string get_block_name(const IR::BasicBlock&);

  void print_live_info(const std::unordered_set<IR::Value>&);

 protected:
  void process_block(IR::Function&, IR::BasicBlock&) override;

  void before_function(IR::Function&) override;

  void after_function(IR::Function&) override;

 public:
  PrintPass(PassManager&, std::ostream&, const PrintPassConfig& = {});
};
}  // namespace Passes
