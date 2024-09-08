#pragma once
#include <memory>

#include "compilation/CompileTreeNodes.h"
#include "syntax/buffalo/SyntaxNode.h"

class CompileTreeBuildStage {
public:
  using input = std::unique_ptr<SyntaxNode>;
  using output = std::unique_ptr<Compilation::ProgramNode>;
  constexpr static auto name = "compile_tree_build";

  std::unique_ptr<Compilation::ProgramNode> apply(std::unique_ptr<SyntaxNode> ast);
};
