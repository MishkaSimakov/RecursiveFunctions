#include "RegisterAllocator.h"

#include "DependenciesGraphBuilder.h"

void IR::RegisterAllocator::apply_to_function(Function& function) {
  DependenciesGraphBuilder builder;

  auto graph = builder(function);
}

void IR::RegisterAllocator::apply(Program& program) {
  // every function is processed separately
  for (auto& function : program.functions) {
    apply_to_function(function);
  }
}
