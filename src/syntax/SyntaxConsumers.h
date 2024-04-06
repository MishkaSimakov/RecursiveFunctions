#ifndef SYNTAXCONSUMERS_H
#define SYNTAXCONSUMERS_H

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "lexis/LexicalAnalyzer.h"
#include "log/Logger.h"

using std::string, std::vector, std::cout, std::endl, std::unique_ptr;

namespace SyntaxConsumers {
class ConcatenatedConsumer;
class CombinedConsumer;
class Consumer;

using RuleIdentifierT = std::size_t;
using GrammarRulesT =
    std::unordered_map<RuleIdentifierT, std::unique_ptr<Consumer>>;

// nodes for consumption tree building
struct RuleConsumptionNode;
struct TokenConsumptionNode;
struct RootConsumptionNode;

class Visitor {
 public:
  virtual void visit(RuleConsumptionNode&) {}
  virtual void visit(TokenConsumptionNode&) {}
  virtual void visit(RootConsumptionNode&) {}

  virtual ~Visitor() = default;
};

struct ConsumptionNode {
  vector<unique_ptr<ConsumptionNode>> children;
  virtual void accept(Visitor&) = 0;
  virtual ~ConsumptionNode() = default;
};

struct RuleConsumptionNode : ConsumptionNode {
  RuleIdentifierT rule;

  explicit RuleConsumptionNode(RuleIdentifierT rule) : rule(std::move(rule)) {}

  void accept(Visitor& visitor) override { visitor.visit(*this); }
};

struct TokenConsumptionNode : ConsumptionNode {
  Token token;

  void accept(Visitor& visitor) override { visitor.visit(*this); }
};

struct RootConsumptionNode : ConsumptionNode {
  void accept(Visitor& visitor) override { visitor.visit(*this); }
};

// consumers for program
class Consumer {
 protected:
  using TokenIteratorT = vector<Token>::const_iterator;

 public:
  virtual bool consume(TokenIteratorT& iterator, const TokenIteratorT& end,
                       const GrammarRulesT& rules,
                       const unique_ptr<ConsumptionNode>& consumption_node) = 0;

  virtual ~Consumer() = default;
};

class ConcatenatedConsumer : public Consumer {
  unique_ptr<Consumer> left_;
  unique_ptr<Consumer> right_;

 public:
  ConcatenatedConsumer(unique_ptr<Consumer> left, unique_ptr<Consumer> right)
      : left_(std::move(left)), right_(std::move(right)) {}

  bool consume(TokenIteratorT& iterator, const TokenIteratorT& end,
               const GrammarRulesT& rules,
               const unique_ptr<ConsumptionNode>& consumption_node) override {
    Logger::syntax("Consuming concatenated 1");

    auto copy = iterator;
    size_t start_children_count = consumption_node->children.size();

    if (!left_->consume(iterator, end, rules, consumption_node)) {
      return false;
    }

    if (!right_->consume(iterator, end, rules, consumption_node)) {
      iterator = copy;
      consumption_node->children.resize(start_children_count);

      return false;
    }

    return true;
  }
};

class CombinedConsumer : public Consumer {
  unique_ptr<Consumer> left_;
  unique_ptr<Consumer> right_;

 public:
  CombinedConsumer(unique_ptr<Consumer> left, unique_ptr<Consumer> right)
      : left_(std::move(left)), right_(std::move(right)) {}

  bool consume(TokenIteratorT& iterator, const TokenIteratorT& end,
               const GrammarRulesT& rules,
               const unique_ptr<ConsumptionNode>& consumation_node) override {
    Logger::syntax("Consuming combined 1");

    if (left_->consume(iterator, end, rules, consumation_node)) {
      return true;
    }

    Logger::syntax("Consuming combined 2");

    return right_->consume(iterator, end, rules, consumation_node);
  }
};

class TokenConsumer : public Consumer {
  TokenType token_;
  string value_;

 public:
  explicit TokenConsumer(TokenType token_type, string value = "")
      : token_(token_type), value_(std::move(value)) {}

  bool consume(TokenIteratorT& iterator, const TokenIteratorT& end,
               const GrammarRulesT& rules,
               const unique_ptr<ConsumptionNode>& consumation_node) override {
    Logger::syntax("Consuming token:", GetTokenDescription({token_, value_}));

    if (iterator == end) {
      Logger::syntax("Met in reality: end of tokens");
      return false;
    }

    Logger::syntax("Met in reality:", GetTokenDescription(*iterator));

    if (iterator->type != token_) {
      return false;
    }

    if (!value_.empty() && iterator->value != value_) {
      return false;
    }

    auto node = std::make_unique<TokenConsumptionNode>();
    node->token = *iterator;
    consumation_node->children.push_back(std::move(node));

    ++iterator;
    return true;
  }
};

class RuleConsumer : public Consumer {
  RuleIdentifierT rule_;

 public:
  explicit RuleConsumer(RuleIdentifierT rule_name)
      : rule_(std::move(rule_name)) {}

  bool consume(TokenIteratorT& iterator, const TokenIteratorT& end,
               const GrammarRulesT& rules,
               const unique_ptr<ConsumptionNode>& consumation_node) override {
    Logger::syntax("Consuming rule:", rule_);

    unique_ptr<ConsumptionNode> node =
        std::make_unique<RuleConsumptionNode>(rule_);

    bool has_consumed = rules.at(rule_)->consume(iterator, end, rules, node);

    if (!has_consumed) {
      return false;
    }

    consumation_node->children.push_back(std::move(node));

    return true;
  }
};

class EmptyConsumer : public Consumer {
 public:
  bool consume(TokenIteratorT& iterator, const TokenIteratorT& end,
               const GrammarRulesT& rules,
               const unique_ptr<ConsumptionNode>& consumation_node) override {
    Logger::syntax("Consuming empty");

    return true;
  }
};

inline auto operator+(unique_ptr<Consumer> left, unique_ptr<Consumer> right) {
  return std::make_unique<ConcatenatedConsumer>(std::move(left),
                                                std::move(right));
}

inline auto operator|(unique_ptr<Consumer> left, unique_ptr<Consumer> right) {
  return std::make_unique<CombinedConsumer>(std::move(left), std::move(right));
}

inline decltype(auto) operator|=(unique_ptr<Consumer>& left,
                                 unique_ptr<Consumer> right) {
  auto combined = std::move(left) | std::move(right);
  left = std::move(combined);
  return left;
}

inline auto EatRule(RuleIdentifierT rule) {
  return std::make_unique<RuleConsumer>(std::move(rule));
}

inline auto EatToken(TokenType token, string value = "") {
  return std::make_unique<TokenConsumer>(token, std::move(value));
}

inline auto EatEmpty() { return std::make_unique<EmptyConsumer>(); }
}  // namespace SyntaxConsumers

#endif  // SYNTAXCONSUMERS_H
