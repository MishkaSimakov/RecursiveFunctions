#pragma once
#include <vector>

#include "lexis/LexicalAnalyzer.h"
#include "syntax/SyntaxNode.h"

class SyntaxTreeBuildStage {
 public:
  using input = const std::vector<Lexing::Token>&;
  using output = std::unique_ptr<SyntaxNode>;
  constexpr static auto name = "syntax_tree_build";

  std::unique_ptr<SyntaxNode> apply(const std::vector<Lexing::Token>& tokens);
};
