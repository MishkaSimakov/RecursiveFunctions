#pragma once

#include <typeindex>
#include <unordered_map>

#include "Analyser.h"
#include "intermediate_representation/BasicBlock.h"
#include "intermediate_representation/Program.h"

namespace Analysis {
class AnalysisManager {
  std::unordered_map<std::type_index, std::unique_ptr<Analyser>> analysers;

 public:
  template <typename T>
    requires std::is_base_of_v<Analyser, T>
  T& getAnalysis(const IR::Program& program) {
    auto itr = analysers.find(typeid(T));

    if (itr == analysers.end()) {
      std::tie(itr, std::ignore) =
          analysers.emplace(typeid(T), std::make_unique<T>());
    }

    auto& analyser = *itr->second;

    if (!analyser.is_valid) {
      analyser.apply(program);
    }

    return static_cast<T&>(analyser);
  }
};
}  // namespace Analysis
