#pragma once
#include "intermediate_representation/Program.h"
#include "passes/Pass.h"
#include "passes/PassManager.h"

namespace Passes {
class ModuleLevelPass : public BasePass {
 public:
  using BasePass::BasePass;

 private:
  void base_apply(IR::Program& program) override {
    bool repeat = get_info().repeat_while_changing;

    bool was_changed;

    do {
      was_changed = apply(program);
      if (was_changed) {
        manager_.invalidate();
      }
    } while (was_changed && repeat);
  }

 protected:
  virtual bool apply(IR::Program& program) = 0;
};
}  // namespace Passes
