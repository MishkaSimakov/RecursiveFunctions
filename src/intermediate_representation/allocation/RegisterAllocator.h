#pragma once
#include <ranges>
#include <unordered_map>
#include <vector>
#include <iostream>

#include "TemporaryDependenciesGraph.h"
#include "intermediate_representation/BasicBlock.h"

namespace IR {
class RegisterAllocator {
  constexpr static size_t kRegistersCount = 5;

  std::unordered_map<Temporary, ssize_t> get_colouring(
      const TemporaryDependenciesGraph&) const;

  void apply_to_function(Function&);
  void add_edges_for_phi(const Function&, TemporaryDependenciesGraph&);
  void set_spill_fine(const Function&, TemporaryDependenciesGraph&);

 public:
  void apply(Program&);
};
}  // namespace IR
