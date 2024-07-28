#pragma once
#include "intermediate_representation/BasicBlock.h"
#include "passes/Pass.h"

namespace Passes {
class RandomOrderPass : public Pass {
protected:
  // called for each block in function
  virtual void process_block(IR::Function&, IR::BasicBlock&) = 0;

  // called once for each function before any of process_block calls
  virtual void before_function(IR::Function&) {}

  // called once for each function after every of process_block calls
  virtual void after_function(IR::Function&) {}

public:
  using Pass::Pass;

  void apply() override;
};
}  // namespace Passes
