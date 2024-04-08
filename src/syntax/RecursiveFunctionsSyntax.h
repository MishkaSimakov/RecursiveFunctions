#ifndef RECURSIVEFUNCTIONSYNTAX_H
#define RECURSIVEFUNCTIONSYNTAX_H

#include "syntax/buffalo/SyntaxConsumers.h"

namespace RecursiveFunctionsSyntax {
using namespace SyntaxConsumers;

enum RuleIdentifiers {
  PROGRAM,
  STATEMENT,
  FUNCTION_VALUE,
  ARGUMENTS_LIST,
  NONEMPTY_ARGUMENTS_LIST,
  COMPOSITION_ARGUMENTS,
  NONEMPTY_COMPOSITION_ARGUMENTS,
  RECURSION_PARAMETER
};

inline unique_ptr<SyntaxNode> PrintParams(const TreeBuilderParams& parameters) {
  for (auto& v : parameters) {
    cout << "[" << static_cast<size_t>(v->type) << " " << v->value << "] ";
  }
  cout << endl;

  return nullptr;
}

inline SyntaxNode ConstructRecursionParameterNode(
    const TreeBuilderParams& params) {
  return SyntaxNode(SyntaxNodeType::RECURSION_PARAMETER, params[0]->value);
}

inline SyntaxNode ConstructAsteriskNode(const TreeBuilderParams&) {
  return SyntaxNode(SyntaxNodeType::ASTERISK);
}

inline SyntaxNode ConstructConstantNode(const TreeBuilderParams& params) {
  return SyntaxNode(SyntaxNodeType::CONSTANT, params[0]->value);
}

inline SyntaxNode ConstructVariableNode(const TreeBuilderParams& params) {
  return SyntaxNode(SyntaxNodeType::VARIABLE, params[0]->value);
}

inline unique_ptr<SyntaxNode> ConstructFunctionNode(TreeBuilderParams& params) {
  auto node =
      std::make_unique<SyntaxNode>(SyntaxNodeType::FUNCTION, params[0]->value);
  node->children = std::move(params[2]->children);

  return node;
}

auto GetNodeConstructor(auto&&... args) {
  return [&](TreeBuilderParams&) {
    return std::make_unique<SyntaxNode>(std::forward<decltype(args)>(args)...);
  };
}

auto GetFirstParamNodeConstructor(auto&&... args) {
  return [&](const TreeBuilderParams& params) {
    return std::make_unique<SyntaxNode>(std::forward<decltype(args)>(args)...,
                                        params[0]->value);
  };
}

auto CompactList(auto&& lambda) {
  return [&](TreeBuilderParams& params) {
    unique_ptr<SyntaxNode> node = lambda(params);
    auto& children = node->children;
    auto& other_children = params.back()->children;
    children.insert(children.end(),
                    std::make_move_iterator(other_children.begin()),
                    std::make_move_iterator(other_children.end()));

    return node;
  };
}

inline auto GetSyntax() {
  GrammarRulesT rules;

  // clang-format off
  rules[PROGRAM] |= Branch(EatRule(STATEMENT) + EatToken(TokenType::SEMICOLON) + EatRule(PROGRAM), PrintParams);
  rules[PROGRAM] |= Branch(EatEmpty(), PrintParams);

  rules[FUNCTION_VALUE] |= Branch(EatToken(TokenType::IDENTIFIER) + EatToken(TokenType::LPAREN) + EatRule(COMPOSITION_ARGUMENTS) + EatToken(TokenType::RPAREN), PrintParams);
  rules[FUNCTION_VALUE] |= Branch(EatToken(TokenType::CONSTANT), PrintParams);
  rules[FUNCTION_VALUE] |= Branch(EatToken(TokenType::IDENTIFIER), PrintParams);

  rules[STATEMENT] |= Branch(EatToken(TokenType::IDENTIFIER) + EatToken(TokenType::LPAREN) + EatRule(ARGUMENTS_LIST) + EatToken(TokenType::RPAREN) + EatToken(TokenType::OPERATOR, "=") + EatRule(FUNCTION_VALUE), PrintParams);

  rules[ARGUMENTS_LIST] |= Branch(EatRule(NONEMPTY_ARGUMENTS_LIST), PrintParams);
  rules[ARGUMENTS_LIST] |= Branch( EatEmpty(), PrintParams);

  rules[NONEMPTY_ARGUMENTS_LIST] |= Branch(EatToken(TokenType::IDENTIFIER) + EatToken(TokenType::COMMA) + EatRule(NONEMPTY_ARGUMENTS_LIST), PrintParams);
  rules[NONEMPTY_ARGUMENTS_LIST] |= Branch(EatRule(RECURSION_PARAMETER), PrintParams);
  rules[NONEMPTY_ARGUMENTS_LIST] |= Branch(EatToken(TokenType::IDENTIFIER), PrintParams);

  rules[COMPOSITION_ARGUMENTS] |= Branch(EatRule(NONEMPTY_COMPOSITION_ARGUMENTS), PrintParams);
  rules[COMPOSITION_ARGUMENTS] |= Branch(EatEmpty());

  rules[NONEMPTY_COMPOSITION_ARGUMENTS] |= Branch(EatToken(TokenType::IDENTIFIER) + EatToken(TokenType::LPAREN) + EatRule(COMPOSITION_ARGUMENTS) + EatToken(TokenType::RPAREN) + EatToken(TokenType::COMMA) + EatRule(NONEMPTY_COMPOSITION_ARGUMENTS), PrintParams);
  rules[NONEMPTY_COMPOSITION_ARGUMENTS] |= Branch(EatToken(TokenType::IDENTIFIER) + EatToken(TokenType::COMMA) + EatRule(NONEMPTY_COMPOSITION_ARGUMENTS), CompactList(GetFirstParamNodeConstructor(SyntaxNodeType::VARIABLE)));
  rules[NONEMPTY_COMPOSITION_ARGUMENTS] |= Branch(EatToken(TokenType::CONSTANT) + EatToken(TokenType::COMMA) + EatRule(NONEMPTY_COMPOSITION_ARGUMENTS), CompactList(GetFirstParamNodeConstructor(SyntaxNodeType::CONSTANT)));
  rules[NONEMPTY_COMPOSITION_ARGUMENTS] |= Branch(EatToken(TokenType::ASTERISK) + EatToken(TokenType::COMMA) + EatRule(NONEMPTY_COMPOSITION_ARGUMENTS), CompactList(GetNodeConstructor(SyntaxNodeType::ASTERISK)));

  rules[NONEMPTY_COMPOSITION_ARGUMENTS] |= Branch(EatToken(TokenType::IDENTIFIER) + EatToken(TokenType::LPAREN) + EatRule(COMPOSITION_ARGUMENTS) + EatToken(TokenType::RPAREN), ConstructFunctionNode);
  rules[NONEMPTY_COMPOSITION_ARGUMENTS] |= Branch(EatToken(TokenType::IDENTIFIER), GetFirstParamNodeConstructor(SyntaxNodeType::VARIABLE));
  rules[NONEMPTY_COMPOSITION_ARGUMENTS] |= Branch(EatToken(TokenType::CONSTANT), GetFirstParamNodeConstructor(SyntaxNodeType::CONSTANT));
  rules[NONEMPTY_COMPOSITION_ARGUMENTS] |= Branch(EatToken(TokenType::ASTERISK), GetNodeConstructor(SyntaxNodeType::ASTERISK));

  rules[RECURSION_PARAMETER] |= Branch(EatToken(TokenType::IDENTIFIER) + EatToken(TokenType::OPERATOR, "+") + EatToken(TokenType::CONSTANT, "1"), GetFirstParamNodeConstructor(SyntaxNodeType::RECURSION_PARAMETER));
  rules[RECURSION_PARAMETER] |= Branch(EatToken(TokenType::CONSTANT, "0"), GetFirstParamNodeConstructor(SyntaxNodeType::CONSTANT));
  // clang-format on

  return rules;
}
}  // namespace RecursiveFunctionsSyntax

#endif  // RECURSIVEFUNCTIONSYNTAX_H
