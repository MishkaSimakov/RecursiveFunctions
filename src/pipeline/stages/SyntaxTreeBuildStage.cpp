#include "SyntaxTreeBuildStage.h"

#include <syntax/RecursiveFunctionsGrammar.h>
#include <syntax/lr/LRParser.h>

#include "utils/Constants.h"

std::unique_ptr<SyntaxNode> SyntaxTreeBuildStage::apply(
    const std::vector<Lexing::Token>& tokens) {
  auto [builders, _] = Syntax::get_recursive_functions_grammar();
  auto parser = Syntax::LRParser(Constants::grammar_filepath, builders);
  auto syntax_tree = parser.parse(tokens);

  return std::move(*syntax_tree);
}
