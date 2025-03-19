#pragma once

#include <vector>

#include "intermediate_representation/Function.h"
#include "passes/Pass.h"
#include "passes/PassManager.h"

namespace Passes {
struct BasicBasicBlocksOrder {
  std::vector<IR::BasicBlock*> operator()(IR::Function& function) const {
    std::vector<IR::BasicBlock*> result;

    for (auto& block : function.basic_blocks) {
      result.push_back(&block);
    }

    return result;
  }
};

struct ReversedPostBasicBlocksOrder {
  std::vector<IR::BasicBlock*> operator()(IR::Function& function) const {
    std::vector<IR::BasicBlock*> result;

    function.reversed_postorder_traversal(
        [&result](IR::BasicBlock* block) { result.push_back(block); });

    return result;
  }
};

template <typename Order = BasicBasicBlocksOrder>
class BasicBlockLevelPass : public BasePass {
 public:
  using BasePass::BasePass;

 private:
  [[no_unique_address]] Order order_;

  void base_apply(IR::Program& program) override {
    bool repeat = get_info().repeat_while_changing;

    for (auto& function : program.functions) {
      before_function(function);

      auto ordering = order_(function);

      for (IR::BasicBlock* block : ordering) {
        bool was_changed;

        do {
          was_changed = apply(function, *block);

          if (was_changed) {
            // TODO: make better, invalidate only this block analysis
            manager_.invalidate();
            function.simplify_blocks();
            function.finalize();
          }
        } while (was_changed && repeat);
      }

      after_function(function);
    }
  }

 protected:
  virtual void before_function(const IR::Function& function) const {}

  virtual bool apply(IR::Function& function, IR::BasicBlock& block) = 0;

  virtual void after_function(const IR::Function& function) const {}
};
}  // namespace Passes
