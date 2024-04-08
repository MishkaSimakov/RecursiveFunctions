#ifndef ADDITIONSYNTAX_H
#define ADDITIONSYNTAX_H

#include "buffalo/SyntaxConsumers.h"
#include "buffalo/SyntaxNode.h"

// This grammar is used for testing and demonstration

namespace AdditionSyntax {
using namespace SyntaxConsumers;

enum RuleIdentifiers {
  EXPRESSION,
};

inline std::unique_ptr<SyntaxNode> ConstructPlusNode(
    TreeBuilderParams& parameters) {
  auto node = std::make_unique<SyntaxNode>(SyntaxNodeType::ASSIGNMENT, "+");
  node->children.push_back(std::move(parameters[0]));

  if (parameters[2]->type == SyntaxNodeType::VARIABLE) {
    node->children.push_back(std::move(parameters[2]));
  } else {
    node->children.insert(
        node->children.end(),
        std::make_move_iterator(parameters[2]->children.begin()),
        std::make_move_iterator(parameters[2]->children.end()));
  }

  return node;
}

inline auto GetFirstParamNodeConstructor(SyntaxNodeType node_type) {
  return [node_type](const TreeBuilderParams& params) {
    return std::make_unique<SyntaxNode>(node_type, params[0]->value);
  };
}

inline auto GetSyntax() {
  GrammarRulesT rules;

  // clang-format off
  rules[EXPRESSION] |= Branch(EatToken(TokenType::IDENTIFIER) + EatToken(TokenType::OPERATOR) + EatRule(EXPRESSION), ConstructPlusNode);
  rules[EXPRESSION] |= Branch(EatToken(TokenType::IDENTIFIER), GetFirstParamNodeConstructor(SyntaxNodeType::VARIABLE));
  // clang-format on

  return rules;
}
}  // namespace AdditionSyntax

#endif  // ADDITIONSYNTAX_H
