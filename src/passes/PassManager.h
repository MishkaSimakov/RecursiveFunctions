#pragma once
#include <functional>
#include <memory>
#include <typeindex>
#include <utility>
#include <vector>

#include "Pass.h"
#include "analysis/Analyser.h"
#include "analysis/liveness/LiveTemporariesStorage.h"
#include "intermediate_representation/BasicBlock.h"
#include "intermediate_representation/Program.h"

namespace Passes {
class PassManager {
 private:
  using PassFactoryT = std::function<std::unique_ptr<Pass>(PassManager&)>;
  std::vector<PassFactoryT> pass_factories_;

  std::unordered_map<std::type_index, std::unique_ptr<Analyser>> analysers;

 public:
  IR::Program& program;

  PassManager(IR::Program& program) : program(program) {}

  template <typename T, typename... Args>
    requires std::is_base_of_v<Pass, T> &&
             std::is_constructible_v<T, PassManager&, Args...>
  void register_pass(Args&&... args) {
    pass_factories_.push_back([&args...](PassManager& manager) {
      return std::make_unique<T>(manager, std::forward<Args>(args)...);
    });
  }

  template <typename T>
    requires std::is_base_of_v<Analyser, T>
  T& get_analysis() {
    auto itr = analysers.find(typeid(T));

    if (itr == analysers.end()) {
      std::tie(itr, std::ignore) =
          analysers.emplace(typeid(T), std::make_unique<T>(*this));
    }

    itr->second->analyse(program);

    return static_cast<T&>(*itr->second);
  }

  void invalidate() {
    for (auto& analyser : analysers | std::views::values) {
      analyser->invalidate();
    }
  }

  void apply();
};
}  // namespace Passes
