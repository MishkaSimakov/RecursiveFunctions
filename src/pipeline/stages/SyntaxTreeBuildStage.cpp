#include "SyntaxTreeBuildStage.h"

#include "syntax/RecursiveFunctionsSyntax.h"
#include "syntax/buffalo/SyntaxTreeBuilder.h"

std::unique_ptr<SyntaxNode> SyntaxTreeBuildStage::apply(
    const std::vector<Token>& tokens) {
  return SyntaxTreeBuilder::build(
      tokens, RecursiveFunctionsSyntax::GetSyntax(),
      RecursiveFunctionsSyntax::RuleIdentifiers::PROGRAM);
}
