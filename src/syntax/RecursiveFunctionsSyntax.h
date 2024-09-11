#pragma once

#include "syntax/buffalo/SyntaxConsumers.h"

namespace RecursiveFunctionsSyntax {
using namespace SyntaxConsumers;

// non-terminals list
enum RuleIdentifiers {
  PROGRAM,
  STATEMENT,

  FUNCTION_DEFINITION,
  EXTERN_FUNCTION_DECLARATION,

  FUNCTION_VALUE,
  ARGUMENTS_LIST,
  NONEMPTY_ARGUMENTS_LIST,
  COMPOSITION_ARGUMENTS,
  NONEMPTY_COMPOSITION_ARGUMENTS,
  RECURSION_PARAMETER,

  FUNCTION_CALL,
  CALL_ARGUMENTS,
  NONEMPTY_CALL_ARGUMENTS,

  SYNTAX_ERROR
};

// rules for building syntax tree
inline auto GetNodeConstructor(SyntaxNodeType node_type) {
  return [node_type](TreeBuilderParams&) {
    return std::make_unique<SyntaxNode>(node_type);
  };
}

inline auto GetFirstParamNodeBuilder(SyntaxNodeType node_type) {
  return [node_type](const TreeBuilderParams& params) {
    return std::make_unique<SyntaxNode>(node_type, params[0]->value);
  };
}

auto GetListRootBuilder(auto&& lambda) {
  return [lambda](TreeBuilderParams& params) {
    auto root_node = std::make_unique<SyntaxNode>(SyntaxNodeType::ROOT);

    root_node->children.push_back(std::move(lambda(params)));

    return root_node;
  };
}

auto CompactList(auto&& lambda) {
  return [lambda](TreeBuilderParams& params) {
    auto element_node = lambda(params);
    auto root_node = std::move(params.back());

    root_node->children.insert(root_node->children.begin(),
                               std::move(element_node));

    return root_node;
  };
}

inline auto BuildProgramNode(TreeBuilderParams& params) {
  auto node = std::move(params[2]);

  if (node == nullptr) {
    node = std::make_unique<SyntaxNode>(SyntaxNodeType::ROOT);
  }

  node->children.insert(node->children.begin(), std::move(params[0]));
  return node;
}

inline auto BuildFunctionDefinitionNode(TreeBuilderParams& params) {
  auto assignment_node =
      std::make_unique<SyntaxNode>(SyntaxNodeType::ASSIGNMENT);

  // build left function node
  auto function_node = std::move(params[2]);

  if (function_node == nullptr) {
    function_node = std::make_unique<SyntaxNode>(SyntaxNodeType::FUNCTION);
  } else {
    function_node->type = SyntaxNodeType::FUNCTION;
  }

  function_node->value = params[0]->value;

  // connect to assignment
  assignment_node->children.push_back(std::move(function_node));
  assignment_node->children.push_back(std::move(params[5]));

  return assignment_node;
}

inline auto BuildExternFunctionDeclarationNode(TreeBuilderParams& params) {
  // build left function node
  auto function_node = std::move(params[5]);

  if (function_node == nullptr) {
    function_node = std::make_unique<SyntaxNode>(SyntaxNodeType::FUNCTION);
  } else {
    function_node->type = SyntaxNodeType::FUNCTION;
  }

  function_node->value = params[3]->value;

  // wrap function into extern specifier
  auto extern_node =
      std::make_unique<SyntaxNode>(SyntaxNodeType::EXTERN_SPECIFIER);

  extern_node->children.push_back(std::move(function_node));
  extern_node->value = "extern";

  return extern_node;
}

inline auto BuildFunctionNode(TreeBuilderParams& params) {
  auto function_node = std::move(params[2]);

  if (function_node == nullptr) {
    function_node = std::make_unique<SyntaxNode>(SyntaxNodeType::FUNCTION);
  } else {
    function_node->type = SyntaxNodeType::FUNCTION;
  }

  function_node->value = params[0]->value;

  return function_node;
}

inline auto Handover(TreeBuilderParams& params) { return std::move(params[0]); }

// rules for parsing and building parsing tree
inline auto GetSyntax() {
  GrammarRulesT rules;

  // clang-format off
  rules[PROGRAM] |= Branch(EatRule(STATEMENT) + EatToken(TokenType::SEMICOLON) + EatRule(PROGRAM), BuildProgramNode);
  rules[PROGRAM] |= Branch(EatEmpty());

  rules[STATEMENT] |= Branch(EatRule(FUNCTION_DEFINITION), Handover);
  rules[STATEMENT] |= Branch(EatRule(EXTERN_FUNCTION_DECLARATION), Handover);

  rules[EXTERN_FUNCTION_DECLARATION] |= Branch(EatToken(TokenType::LPAREN) + EatToken(TokenType::IDENTIFIER, "extern") + EatToken(TokenType::RPAREN) + EatToken(TokenType::IDENTIFIER) + EatToken(TokenType::LPAREN) + EatRule(ARGUMENTS_LIST) + EatToken(TokenType::RPAREN), BuildExternFunctionDeclarationNode);

  rules[FUNCTION_DEFINITION] |= Branch(EatToken(TokenType::IDENTIFIER) + EatToken(TokenType::LPAREN) + EatRule(ARGUMENTS_LIST) + EatToken(TokenType::RPAREN) + EatToken(TokenType::OPERATOR, "=") + EatRule(FUNCTION_VALUE), BuildFunctionDefinitionNode);

  rules[FUNCTION_VALUE] |= Branch(EatToken(TokenType::IDENTIFIER) + EatToken(TokenType::LPAREN) + EatRule(COMPOSITION_ARGUMENTS) + EatToken(TokenType::RPAREN), BuildFunctionNode);
  rules[FUNCTION_VALUE] |= Branch(EatToken(TokenType::CONSTANT), GetFirstParamNodeBuilder(SyntaxNodeType::CONSTANT));
  rules[FUNCTION_VALUE] |= Branch(EatToken(TokenType::IDENTIFIER), GetFirstParamNodeBuilder(SyntaxNodeType::VARIABLE));

  rules[ARGUMENTS_LIST] |= Branch(EatRule(NONEMPTY_ARGUMENTS_LIST), Handover);
  rules[ARGUMENTS_LIST] |= Branch(EatEmpty());

  rules[NONEMPTY_ARGUMENTS_LIST] |= Branch(EatToken(TokenType::IDENTIFIER) + EatToken(TokenType::COMMA) + EatRule(NONEMPTY_ARGUMENTS_LIST), CompactList(GetFirstParamNodeBuilder(SyntaxNodeType::VARIABLE)));
  rules[NONEMPTY_ARGUMENTS_LIST] |= Branch(EatRule(RECURSION_PARAMETER), Handover);
  rules[NONEMPTY_ARGUMENTS_LIST] |= Branch(EatToken(TokenType::IDENTIFIER), GetListRootBuilder(GetFirstParamNodeBuilder(SyntaxNodeType::VARIABLE)));

  rules[COMPOSITION_ARGUMENTS] |= Branch(EatRule(NONEMPTY_COMPOSITION_ARGUMENTS), Handover);
  rules[COMPOSITION_ARGUMENTS] |= Branch(EatEmpty());

  rules[NONEMPTY_COMPOSITION_ARGUMENTS] |= Branch(EatToken(TokenType::IDENTIFIER) + EatToken(TokenType::LPAREN) + EatRule(COMPOSITION_ARGUMENTS) + EatToken(TokenType::RPAREN) + EatToken(TokenType::COMMA) + EatRule(NONEMPTY_COMPOSITION_ARGUMENTS), CompactList(BuildFunctionNode));
  rules[NONEMPTY_COMPOSITION_ARGUMENTS] |= Branch(EatToken(TokenType::IDENTIFIER) + EatToken(TokenType::COMMA) + EatRule(NONEMPTY_COMPOSITION_ARGUMENTS), CompactList(GetFirstParamNodeBuilder(SyntaxNodeType::VARIABLE)));
  rules[NONEMPTY_COMPOSITION_ARGUMENTS] |= Branch(EatToken(TokenType::CONSTANT) + EatToken(TokenType::COMMA) + EatRule(NONEMPTY_COMPOSITION_ARGUMENTS), CompactList(GetFirstParamNodeBuilder(SyntaxNodeType::CONSTANT)));
  rules[NONEMPTY_COMPOSITION_ARGUMENTS] |= Branch(EatToken(TokenType::ASTERISK) + EatToken(TokenType::COMMA) + EatRule(NONEMPTY_COMPOSITION_ARGUMENTS), CompactList(GetNodeConstructor(SyntaxNodeType::ASTERISK)));

  rules[NONEMPTY_COMPOSITION_ARGUMENTS] |= Branch(EatToken(TokenType::IDENTIFIER) + EatToken(TokenType::LPAREN) + EatRule(COMPOSITION_ARGUMENTS) + EatToken(TokenType::RPAREN), GetListRootBuilder(BuildFunctionNode));
  rules[NONEMPTY_COMPOSITION_ARGUMENTS] |= Branch(EatToken(TokenType::IDENTIFIER), GetListRootBuilder(GetFirstParamNodeBuilder(SyntaxNodeType::VARIABLE)));
  rules[NONEMPTY_COMPOSITION_ARGUMENTS] |= Branch(EatToken(TokenType::CONSTANT), GetListRootBuilder(GetFirstParamNodeBuilder(SyntaxNodeType::CONSTANT)));
  rules[NONEMPTY_COMPOSITION_ARGUMENTS] |= Branch(EatToken(TokenType::ASTERISK), GetListRootBuilder(GetFirstParamNodeBuilder(SyntaxNodeType::ASTERISK)));

  rules[RECURSION_PARAMETER] |= Branch(EatToken(TokenType::IDENTIFIER) + EatToken(TokenType::OPERATOR, "+") + EatToken(TokenType::CONSTANT, "1"), GetListRootBuilder(GetFirstParamNodeBuilder(SyntaxNodeType::RECURSION_PARAMETER)));
  rules[RECURSION_PARAMETER] |= Branch(EatToken(TokenType::CONSTANT, "0"), GetListRootBuilder(GetFirstParamNodeBuilder(SyntaxNodeType::RECURSION_PARAMETER)));

  rules[FUNCTION_CALL] |= Branch(EatToken(TokenType::IDENTIFIER) + EatToken(TokenType::LPAREN) + EatRule(CALL_ARGUMENTS) + EatToken(TokenType::RPAREN), BuildFunctionNode);

  rules[CALL_ARGUMENTS] |= Branch(EatRule(NONEMPTY_CALL_ARGUMENTS), Handover);
  rules[CALL_ARGUMENTS] |= Branch(EatEmpty());

  rules[NONEMPTY_CALL_ARGUMENTS] |= Branch(EatToken(TokenType::IDENTIFIER) + EatToken(TokenType::LPAREN) + EatRule(CALL_ARGUMENTS) + EatToken(TokenType::RPAREN) + EatToken(TokenType::COMMA) + EatRule(NONEMPTY_CALL_ARGUMENTS), CompactList(BuildFunctionNode));
  rules[NONEMPTY_CALL_ARGUMENTS] |= Branch(EatToken(TokenType::CONSTANT) + EatToken(TokenType::COMMA) + EatRule(NONEMPTY_CALL_ARGUMENTS), CompactList(GetFirstParamNodeBuilder(SyntaxNodeType::CONSTANT)));
  rules[NONEMPTY_CALL_ARGUMENTS] |= Branch(EatToken(TokenType::ASTERISK) + EatToken(TokenType::COMMA) + EatRule(NONEMPTY_CALL_ARGUMENTS), CompactList(GetFirstParamNodeBuilder(SyntaxNodeType::ASTERISK)));

  rules[NONEMPTY_CALL_ARGUMENTS] |= Branch(EatToken(TokenType::IDENTIFIER) + EatToken(TokenType::LPAREN) + EatRule(CALL_ARGUMENTS) + EatToken(TokenType::RPAREN), GetListRootBuilder(BuildFunctionNode));
  rules[NONEMPTY_CALL_ARGUMENTS] |= Branch(EatToken(TokenType::CONSTANT), GetListRootBuilder(GetFirstParamNodeBuilder(SyntaxNodeType::CONSTANT)));
  rules[NONEMPTY_CALL_ARGUMENTS] |= Branch(EatToken(TokenType::ASTERISK), GetListRootBuilder(GetFirstParamNodeBuilder(SyntaxNodeType::ASTERISK)));
  // clang-format on

  return rules;
}
}  // namespace RecursiveFunctionsSyntax
