#pragma once
#include "passes/analysis/Analyser.h"

namespace Passes {
class EqualitiesAnalysis : public Analyser {
 public:
  using Analyser::Analyser;

  const auto& get_equalities(const IR::Function& function) const {
    return equalities_.at(function.name);
  }

 protected:
  std::unordered_map<std::string, std::unordered_map<IR::Value, IR::Value>>
      equalities_;

  void perform_analysis(const IR::Program&) override;
};
}  // namespace Passes
