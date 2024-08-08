#pragma once
#include <Constants.h>

#include <array>
#include <optional>
#include <unordered_set>
#include <vector>

#include "passes/Pass.h"
#include "passes/PassManager.h"

namespace Passes {
class RegisterAllocationPass : public Pass {
  constexpr static size_t kMaxRegistersAllowed = 16;

  struct TemporaryInfo {
    IR::Value virtual_register;

    std::vector<const IR::FunctionCall*> inside_lifetime;
    std::vector<const IR::FunctionCall*> on_lifetime_edge;

    // only for basic register, for finding best register
    std::array<size_t, kMaxRegistersAllowed> registers_usage{0};

    // negatives for callee-saved, positive for basic
    std::optional<IR::Value> assigned_register;

    std::unordered_set<IR::Value> dependencies;

    bool is_basic() const { return inside_lifetime.empty(); }

    bool is_callee_saved() const { return !is_basic(); }
  };

  std::unordered_map<IR::Value, TemporaryInfo> vregs_info;

  void assign_registers(
      const std::vector<std::pair<IR::Value, TemporaryInfo*>>&,
      IR::ValueType) const;

  void apply_transformation(
      IR::Function&, const std::unordered_map<IR::Value, TemporaryInfo>&);

 public:
  using Pass::Pass;

  void apply() override;
};
}  // namespace Passes
