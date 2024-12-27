#include "RecursiveFunctionsGrammar.h"

namespace {
std::unique_ptr<SyntaxNode> Pass(std::span<std::unique_ptr<SyntaxNode>> nodes) {
  return nullptr;
}

auto GetNodeConstructor(SyntaxNodeType node_type) {
  return [node_type](std::span<std::unique_ptr<SyntaxNode>>) {
    return std::make_unique<SyntaxNode>(node_type);
  };
}

auto GetFirstParamNodeBuilder(SyntaxNodeType node_type) {
  return [node_type](std::span<std::unique_ptr<SyntaxNode>> nodes) {
    return std::make_unique<SyntaxNode>(node_type, nodes[0]->value);
  };
}

auto GetListRootBuilder(auto&& lambda) {
  return [lambda](std::span<std::unique_ptr<SyntaxNode>> nodes) {
    auto root_node = std::make_unique<SyntaxNode>(SyntaxNodeType::ROOT);

    root_node->children.push_back(std::move(lambda(nodes)));

    return root_node;
  };
}

auto CompactList(auto&& lambda) {
  return [lambda](std::span<std::unique_ptr<SyntaxNode>> nodes) {
    auto element_node = lambda(nodes);
    auto root_node = std::move(nodes.back());

    root_node->children.insert(root_node->children.begin(),
                               std::move(element_node));

    return root_node;
  };
}

auto BuildProgramNode(std::span<std::unique_ptr<SyntaxNode>> nodes) {
  auto node = std::move(nodes[0]);

  if (node == nullptr) {
    node = std::make_unique<SyntaxNode>(SyntaxNodeType::ROOT);
  }

  node->children.push_back(std::move(nodes[1]));
  return node;
}

auto Handover(size_t index = 0) {
  return [index](std::span<std::unique_ptr<SyntaxNode>> nodes) {
    return std::move(nodes[index]);
  };
}

auto BuildFunctionDefinitionNode(std::span<std::unique_ptr<SyntaxNode>> nodes) {
  auto assignment_node =
      std::make_unique<SyntaxNode>(SyntaxNodeType::ASSIGNMENT);

  // build left function node
  auto function_node = std::move(nodes[1]);
  function_node->type = SyntaxNodeType::FUNCTION;
  function_node->value = nodes[0]->value;

  // connect to assignment
  assignment_node->children.push_back(std::move(function_node));
  assignment_node->children.push_back(std::move(nodes[3]));

  return assignment_node;
}

auto BuildFunctionNode(std::span<std::unique_ptr<SyntaxNode>> nodes) {
  auto function_node = std::move(nodes[1]);

  if (function_node == nullptr) {
    function_node = std::make_unique<SyntaxNode>(SyntaxNodeType::FUNCTION);
  } else {
    function_node->type = SyntaxNodeType::FUNCTION;
  }

  function_node->value = nodes[0]->value;

  return function_node;
}

auto BuildExternFunctionDeclarationNode(
    std::span<std::unique_ptr<SyntaxNode>> nodes) {
  auto function_node = std::move(nodes[4]);
  function_node->type = SyntaxNodeType::FUNCTION;
  function_node->value = nodes[3]->value;

  // wrap function into extern specifier
  auto extern_node =
      std::make_unique<SyntaxNode>(SyntaxNodeType::EXTERN_SPECIFIER);

  extern_node->children.push_back(std::move(function_node));
  extern_node->value = "extern";

  return extern_node;
}
}  // namespace

namespace Syntax {
GrammarBuilder<SyntaxNode, cIsCompilingGrammar>
get_recursive_functions_grammar() {
  using enum Lexis::TokenType::InternalEnum;

  GrammarBuilder<SyntaxNode, cIsCompilingGrammar> builder;

  // clang-format off
  // NOLINTBEGIN

  // non-terminals
  // grammar
  // builder.add(PROGRAM, PROGRAM+ STATEMENT + Terminal{SEMICOLON}, BuildProgramNode);
  // builder.add(PROGRAM, STATEMENT + Terminal{SEMICOLON}, GetListRootBuilder(Handover()));
  //
  // builder.add(STATEMENT, FUNCTION_DEFINITION, Handover());
  // builder.add(STATEMENT, EXTERN_FUNCTION_DECLARATION, Handover());
  //
  // builder.add(EXTERN_FUNCTION_DECLARATION, Terminal{LPAREN} + Terminal{KW_EXTERN} + Terminal{RPAREN} + Terminal{IDENTIFIER} + ARGUMENTS, BuildExternFunctionDeclarationNode);
  //
  // builder.add(FUNCTION_DEFINITION, Terminal{IDENTIFIER} + ARGUMENTS + Terminal{OP_EQUAL} + FUNCTION_VALUE, BuildFunctionDefinitionNode);
  //
  // builder.add(ARGUMENTS, Terminal{LPAREN} + Terminal{RPAREN}, Handover());
  // builder.add(ARGUMENTS, Terminal{LPAREN} + NONEMPTY_ARGUMENTS + Terminal{RPAREN}, Handover(1));
  //
  // builder.add(NONEMPTY_ARGUMENTS, Terminal{IDENTIFIER} + Terminal{COMMA} + NONEMPTY_ARGUMENTS, CompactList(GetFirstParamNodeBuilder(SyntaxNodeType::VARIABLE)));
  // builder.add(NONEMPTY_ARGUMENTS, Terminal{IDENTIFIER}, GetListRootBuilder(GetFirstParamNodeBuilder(SyntaxNodeType::VARIABLE)));
  // builder.add(NONEMPTY_ARGUMENTS, RECURSION_ARGUMENT, Handover());
  //
  // builder.add(RECURSION_ARGUMENT, Terminal{CONSTANT}, GetListRootBuilder(GetFirstParamNodeBuilder(SyntaxNodeType::RECURSION_PARAMETER)));
  // builder.add(RECURSION_ARGUMENT, Terminal{IDENTIFIER} + Terminal{OP_PLUS} + Terminal{CONSTANT}, GetListRootBuilder(GetFirstParamNodeBuilder(SyntaxNodeType::RECURSION_PARAMETER)));
  //
  // builder.add(FUNCTION_VALUE, Terminal{IDENTIFIER} + COMPOSITION_ARGUMENTS , BuildFunctionNode);
  // builder.add(FUNCTION_VALUE, Terminal{CONSTANT}, GetFirstParamNodeBuilder(SyntaxNodeType::CONSTANT));
  // builder.add(FUNCTION_VALUE, Terminal{IDENTIFIER}, GetFirstParamNodeBuilder(SyntaxNodeType::VARIABLE));
  //
  // builder.add(COMPOSITION_ARGUMENTS, Terminal{LPAREN} + NONEMPTY_COMPOSITION_ARGUMENTS + Terminal{RPAREN}, Handover(1));
  // builder.add(COMPOSITION_ARGUMENTS, Terminal{LPAREN} + Terminal{RPAREN}, Handover());
  //
  // builder.add(NONEMPTY_COMPOSITION_ARGUMENTS, Terminal{IDENTIFIER} + COMPOSITION_ARGUMENTS + Terminal{COMMA} + NONEMPTY_COMPOSITION_ARGUMENTS, CompactList(BuildFunctionNode));
  // builder.add(NONEMPTY_COMPOSITION_ARGUMENTS, Terminal{IDENTIFIER} + Terminal{COMMA} + NONEMPTY_COMPOSITION_ARGUMENTS, CompactList(GetFirstParamNodeBuilder(SyntaxNodeType::VARIABLE)));
  // builder.add(NONEMPTY_COMPOSITION_ARGUMENTS, Terminal{CONSTANT} + Terminal{COMMA} + NONEMPTY_COMPOSITION_ARGUMENTS, CompactList(GetFirstParamNodeBuilder(SyntaxNodeType::CONSTANT)));
  // builder.add(NONEMPTY_COMPOSITION_ARGUMENTS, Terminal{ASTERISK} + Terminal{COMMA} + NONEMPTY_COMPOSITION_ARGUMENTS, CompactList(GetNodeConstructor(SyntaxNodeType::ASTERISK)));
  //
  // builder.add(NONEMPTY_COMPOSITION_ARGUMENTS, Terminal{IDENTIFIER} + COMPOSITION_ARGUMENTS, GetListRootBuilder(BuildFunctionNode));
  // builder.add(NONEMPTY_COMPOSITION_ARGUMENTS, Terminal{IDENTIFIER}, GetListRootBuilder(GetFirstParamNodeBuilder(SyntaxNodeType::VARIABLE)));
  // builder.add(NONEMPTY_COMPOSITION_ARGUMENTS, Terminal{CONSTANT}, GetListRootBuilder(GetFirstParamNodeBuilder(SyntaxNodeType::CONSTANT)));
  // builder.add(NONEMPTY_COMPOSITION_ARGUMENTS, Terminal{ASTERISK}, GetListRootBuilder(GetFirstParamNodeBuilder(SyntaxNodeType::ASTERISK)));

  // builder.set_start(PROGRAM);
  // NOLINTEND
  // clang-format on

  return builder;
}
}  // namespace Syntax
