#pragma once
#include <unordered_map>

#include "RegistersInfo.h"
#include "intermediate_representation/Function.h"
#include "intermediate_representation/Value.h"
#include "passes/liveness/LiveTemporariesStorage.h"

namespace Passes {
class CostModel {
 public:
  struct Costs {
    // cost of move operations that are generated near function call
    // TODO: do something with this constant
    std::array<double, 32> moves;
    double spill{0};
    double calee_saved{0};
    double basic{0};
  };

 private:
  static constexpr double kLoadStoreCost = 3;
  static constexpr double kMoveCost = 1;

  std::unordered_map<IR::Value, Costs> costs;
  const LiveTemporariesStorage& liveness;
  const IR::Function& function;

  void process_function_call(const IR::FunctionCall& call);

 public:
  CostModel(const IR::Function& function,
            const LiveTemporariesStorage& liveness)
      : liveness(liveness), function(function) {}

  std::unordered_map<IR::Value, Costs> estimate_costs();
};
}  // namespace Passes
