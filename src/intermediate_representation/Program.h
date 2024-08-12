#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "Function.h"

namespace IR {
struct Program {
 private:
  std::vector<Function> functions;

 public:
  std::span<Function> get_functions() { return functions; }

  void add_function(auto&&... args) {
    Function& function =
        functions.emplace_back(std::forward<decltype(args)>(args)...);

    function.tie_to_program(functions.size() - 1);
  }

  Function& get_function(const std::string& name) {
    auto itr = std::ranges::find_if(
        functions,
        [&name](const Function& function) { return name == function.name; });

    if (itr == functions.end()) {
      throw std::runtime_error(
          fmt::format("Could not find function with name {}", name));
    }

    return *itr;
  }
};
}  // namespace IR
