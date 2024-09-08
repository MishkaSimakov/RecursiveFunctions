#include "CompileTreeBuildStage.h"

#include "compilation/CompileTreeBuilder.h"

std::unique_ptr<Compilation::ProgramNode> CompileTreeBuildStage::apply(
    std::unique_ptr<SyntaxNode> ast) {
  Compilation::CompileTreeBuilder compile_tree_builder;
  return compile_tree_builder.build(*ast);
}
