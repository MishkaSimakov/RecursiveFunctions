// #pragma once
// #include <iostream>
// #include <ranges>
// #include <unordered_map>
// #include <vector>
//
// #include "TemporaryDependenciesGraph.h"
// #include "intermediate_representation/BasicBlock.h"
// #include "intermediate_representation/Function.h"
// #include "intermediate_representation/Program.h"
//
// namespace IR {
// class RegisterAllocator {
//   constexpr static size_t kRegistersCount = 5;
//
//   // weights
//   constexpr static double kPhiNodeWeight = 50;
//   constexpr static double kSpillCost = -1000;
//
//   std::unordered_map<Value, ssize_t> get_colouring(
//       const TemporaryDependenciesGraph&) const;
//
//   void apply_to_function(Function&);
//   void add_edges_for_phi(const Function&, TemporaryDependenciesGraph&);
//   void set_spill_fine(const Function&, TemporaryDependenciesGraph&);
//   void remove_phi_nodes(Function&, const std::unordered_map<Value, ssize_t>&);
//
//  public:
//   void apply(Program&);
// };
// }  // namespace IR
