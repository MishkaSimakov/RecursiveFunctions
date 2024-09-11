#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "Function.h"

namespace IR {
struct Program {
  std::vector<Function> functions;
  std::unordered_set<std::string> extern_functions;

  Function* get_function(const std::string& name) {
    // search for that function in defined functions
    auto itr = std::ranges::find_if(
        functions,
        [&name](const Function& function) { return name == function.name; });

    if (itr != functions.end()) {
      return &(*itr);
    }

    // then search in extern defined functions
    if (extern_functions.contains(name)) {
      return nullptr;
    }

    // if not found then there is error somewhere
    throw std::runtime_error(
        fmt::format("Could not find function with name {}", name));
  }
};
}  // namespace IR
