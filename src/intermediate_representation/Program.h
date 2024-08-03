#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "Function.h"

namespace IR {
struct Program {
  std::vector<Function> functions;

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
