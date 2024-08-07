#pragma once
#include <iostream>

#include "intermediate_representation/Program.h"

namespace Assembly {
class AssemblyPrinter {
  const IR::Program& program_;

 public:
  explicit AssemblyPrinter(const IR::Program& program) : program_(program) {}

  void print(std::ostream&);
};
}  // namespace Assembly
