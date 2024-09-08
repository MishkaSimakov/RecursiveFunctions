#pragma once
#include "intermediate_representation/Function.h"
#include "passes/Pass.h"
#include "passes/PassManager.h"

namespace Passes {
struct BasicFunctionsOrder {
  std::vector<IR::Function*> operator()(IR::Program& program) const {
    std::vector<IR::Function*> result;

    for (auto& function : program.functions) {
      result.push_back(&function);
    }

    return result;
  }
};

template <typename Order = BasicFunctionsOrder>
class FunctionLevelPass : public BasePass {
 public:
  using BasePass::BasePass;

 private:
  [[no_unique_address]] Order order_;

  void base_apply(IR::Program& program) override {
    auto ordering = order_(program);

    bool repeat = get_info().repeat_while_changing;

    for (IR::Function* function : ordering) {
      bool was_changed;
      do {
        was_changed = apply(*function);

        if (was_changed) {
          // TODO: make better, invalidate only this function analysis
          manager_.invalidate();
          function->simplify_blocks();
          function->finalize();
        }
      } while (was_changed && repeat);
    }
  }

 protected:
  virtual bool apply(IR::Function& function) = 0;
};
}  // namespace Passes
