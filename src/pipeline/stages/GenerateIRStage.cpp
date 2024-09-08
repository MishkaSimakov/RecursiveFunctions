#include "GenerateIRStage.h"

#include "intermediate_representation/compiler/IRCompiler.h"

IR::Program GenerateIRStage::apply(
    std::unique_ptr<Compilation::ProgramNode> compile_tree) {
  return IR::IRCompiler().get_ir(*compile_tree);
}
