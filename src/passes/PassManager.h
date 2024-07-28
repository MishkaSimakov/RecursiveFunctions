#pragma once
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "Pass.h"
#include "intermediate_representation/BasicBlock.h"
#include "liveness/LiveTemporariesStorage.h"

namespace Passes {
class PassManager {
 private:
  using PassFactoryT = std::function<std::unique_ptr<Pass>(PassManager&)>;
  std::vector<PassFactoryT> pass_factories_;

 public:
  IR::Program& program;
  LiveTemporariesStorage live_storage;

  PassManager(IR::Program& program): program(program) {}

  template <typename T, typename... Args>
    requires std::is_base_of_v<Pass, T> &&
             std::is_constructible_v<T, PassManager&, Args...>
  void register_pass(Args&&... args) {
    pass_factories_.push_back([&args...](PassManager& manager) {
      return std::make_unique<T>(manager, std::forward<Args>(args)...);
    });
  }

  void apply();
};
}  // namespace Passes
