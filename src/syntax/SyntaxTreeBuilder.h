#ifndef SYNTAXTREEBUILDER_H
#define SYNTAXTREEBUILDER_H

#include <iostream>
#include <string>
#include <vector>

#include "SyntaxConsumers.h"
#include "lexis/LexicalAnalyzer.h"

using std::string, std::vector, std::cout, std::endl, std::unique_ptr;

enum class SyntaxNodeType { VARIABLE, ASTERISK, FUNCTION, ASSIGNMENT };

struct SyntaxNode {
  SyntaxNodeType type;
  string value;
  vector<SyntaxNode> children;

  SyntaxNode(const SyntaxNode& other) {
    std::cout << "copied" << std::endl;

    type = other.type;
    value = other.value;
    children = other.children;
  }
};

class SyntaxTreeBuilder {
  struct DrawTreeVisitor : SyntaxConsumers::Visitor {
    size_t indentation;

    string get_prefix() const {
      string result;

      for (size_t i = 0; i < indentation; ++i) {
        result += "| ";
      }

      return result;
    }

    void visit(SyntaxConsumers::RuleConsumptionNode& node) override {
      cout << get_prefix() << "rule node with rule: " << node.rule << endl;
    }
    void visit(SyntaxConsumers::TokenConsumptionNode& node) override {
      cout << get_prefix()
           << "token node with token: " << GetTokenDescription(node.token)
           << endl;
    }
    void visit(SyntaxConsumers::RootConsumptionNode& node) override {
      cout << get_prefix() << "just root" << endl;
    }
  };

  static void draw(unique_ptr<SyntaxConsumers::ConsumptionNode>& root,
                   DrawTreeVisitor& visitor) {
    root->accept(visitor);

    ++visitor.indentation;
    for (auto& child : root->children) {
      draw(child, visitor);
    }
    --visitor.indentation;
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

    if (has_consumed && position == end) {
      cout << "Success!" << endl;

      auto visitor = DrawTreeVisitor();
      draw(consumption_root, visitor);

      return;
    }

    throw std::runtime_error("No rules matched your program.");
  }
};

#endif  // SYNTAXTREEBUILDER_H
