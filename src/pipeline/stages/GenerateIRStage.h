#pragma once
#include <memory>

#include "compilation/CompileTreeNodes.h"
#include "intermediate_representation/Program.h"

class GenerateIRStage {
 public:
  using input = std::unique_ptr<Compilation::ProgramNode>;
  using output = IR::Program;
  constexpr static auto name = "generate_ir";

  IR::Program apply(std::unique_ptr<Compilation::ProgramNode> compile_tree);
};
