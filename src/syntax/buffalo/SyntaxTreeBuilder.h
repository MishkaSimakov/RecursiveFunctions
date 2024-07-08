#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

#include "Exceptions.h"
#include "SyntaxConsumers.h"
#include "lexis/LexicalAnalyzer.h"

using std::string, std::vector, std::cout, std::endl, std::unique_ptr,
    std::weak_ptr;

class SyntaxTreeBuilder {
  static unique_ptr<SyntaxNode> construct_syntax_tree(
      const unique_ptr<SyntaxConsumers::ConsumptionNode>& node) {
    vector<unique_ptr<SyntaxNode>> built_syntax_nodes;

    for (auto& child : node->children) {
      built_syntax_nodes.push_back(std::move(construct_syntax_tree(child)));
    }

    return node->build_syntax_tree(built_syntax_nodes);
  }

  // please forgive me, this function I've taken from StackOverflow
  static void print_syntax_tree_recursive(const std::string& prefix,
                                          const SyntaxNode& node,
                                          bool is_last) {
    std::cout << prefix;

    std::cout << (is_last ? "└── " : "├── ");

    // print the value of the node
    string result;
    switch (node.type) {
      case SyntaxNodeType::ROOT:
        result = "Root";
        break;
      case SyntaxNodeType::VARIABLE:
        result = "Var{" + node.value + "}";
        break;
      case SyntaxNodeType::CONSTANT:
        result = "Num{" + node.value + "}";
        break;
      case SyntaxNodeType::ASTERISK:
        result = "*";
        break;
      case SyntaxNodeType::FUNCTION:
        result = "Func{" + node.value + "}";
        break;
      case SyntaxNodeType::ASSIGNMENT:
        result = "=";
        break;
      case SyntaxNodeType::RECURSION_PARAMETER:
        result = "Rec{" + node.value + "}";
        break;
    }
    std::cout << result << std::endl;

    // enter the next tree level - left and right branch
    for (size_t i = 0; i < node.children.size(); ++i) {
      print_syntax_tree_recursive(prefix + (is_last ? "    " : "│   "),
                                  *node.children[i],
                                  i == node.children.size() - 1);
    }
  }

  static void print_syntax_tree(const SyntaxNode& node) {
    print_syntax_tree_recursive("", node, true);
  }

 public:
  static unique_ptr<SyntaxNode> build(
      const vector<Token>& program, const SyntaxConsumers::GrammarRulesT& rules,
      const SyntaxConsumers::RuleIdentifierT& start) {
    using namespace SyntaxConsumers;

    Logger::syntax(LogLevel::INFO,
                   "start parsing tokens using context-free grammar");

    auto position = program.begin();
    auto end = program.end();
    auto starting_consumer = RuleConsumer(start);

    unique_ptr<ConsumptionNode> consumption_root =
        std::make_unique<RootConsumptionNode>();

    bool has_consumed =
        starting_consumer.consume(position, end, rules, consumption_root);

    if (!has_consumed || position != end) {
      throw Syntax::NoRulesMatchedException();
    }

    Logger::syntax(LogLevel::INFO, "tokens successfully parsed");
    Logger::syntax(LogLevel::INFO, "start building abstract syntax tree");

    return construct_syntax_tree(consumption_root);
  }
};
