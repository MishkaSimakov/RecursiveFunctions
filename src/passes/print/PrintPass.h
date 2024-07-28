#pragma once
#include <iostream>
#include <unordered_map>

#include "passes/pass_types/ParentsFirstPass.h"

namespace Passes {
class PrintPass : public ParentsFirstPass {
  std::ostream& os_;
  std::unordered_map<const IR::BasicBlock*, size_t> indices_;

 protected:
  void process_block(IR::Function&, IR::BasicBlock&) override;

  void before_function(IR::Function&) override;

  void after_function(IR::Function&) override;

  size_t get_block_index(const IR::BasicBlock&);

 public:
  PrintPass(PassManager&, std::ostream&);
};
}  // namespace Passes
