#pragma once
#include "intermediate_representation/Program.h"

class OptimizeIRStage {
 public:
  using input = IR::Program;
  using output = IR::Program;
  constexpr static auto name = "optimize_ir";

  IR::Program apply(IR::Program program);
};
