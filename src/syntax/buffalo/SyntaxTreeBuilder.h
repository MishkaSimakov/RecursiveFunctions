#ifndef SYNTAXTREEBUILDER_H
#define SYNTAXTREEBUILDER_H

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

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

 public:
  static void build(const vector<Token>& program,
                    const SyntaxConsumers::GrammarRulesT& rules,
                    const SyntaxConsumers::RuleIdentifierT& start) {
    using namespace SyntaxConsumers;

    auto position = program.begin();
    auto end = program.end();
    auto starting_consumer = RuleConsumer(start);

    unique_ptr<ConsumptionNode> consumption_root =
        std::make_unique<RootConsumptionNode>();

    bool has_consumed =
        starting_consumer.consume(position, end, rules, consumption_root);

    if (!has_consumed || position != end) {
      throw std::runtime_error("No rules matched your program.");
    }

    unique_ptr<SyntaxNode> syntax_root = construct_syntax_tree(consumption_root);

    cout << "Success!" << endl;
  }
};

#endif  // SYNTAXTREEBUILDER_H
