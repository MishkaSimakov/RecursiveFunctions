#pragma once

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "lexis/LexicalAnalyzer.h"
#include "log/Logger.h"
#include "syntax/buffalo/SyntaxNode.h"

using namespace Lexing;
using std::string, std::vector, std::cout, std::endl, std::unique_ptr;

namespace SyntaxConsumers {
class BranchedConsumer;
class Consumer;

using RuleIdentifierT = std::size_t;
using GrammarRulesT = std::unordered_map<RuleIdentifierT, BranchedConsumer>;

// nodes for consumption tree building
using TreeBuilderParams = vector<unique_ptr<SyntaxNode>>;

struct ConsumptionNode {
  vector<unique_ptr<ConsumptionNode>> children;
  virtual unique_ptr<SyntaxNode> build_syntax_tree(TreeBuilderParams&) = 0;

  virtual ~ConsumptionNode() = default;
};

struct TokenConsumptionNode final : ConsumptionNode {
  Token token;

  unique_ptr<SyntaxNode> build_syntax_tree(TreeBuilderParams&) override {
    return std::make_unique<SyntaxNode>(SyntaxNodeType::VARIABLE, token.value);
  }
};

struct RootConsumptionNode final : ConsumptionNode {
  unique_ptr<SyntaxNode> build_syntax_tree(TreeBuilderParams& params) override {
    return std::move(params[0]);
  }
};

// consumers for program
class Consumer {
 protected:
  using TokenIteratorT = vector<Token>::const_iterator;

 public:
  virtual bool consume(
      TokenIteratorT& iterator, const TokenIteratorT& end,
      const GrammarRulesT& rules,
      const unique_ptr<ConsumptionNode>& consumption_node) const = 0;

  virtual ~Consumer() = default;
};

class Branch {
  static unique_ptr<SyntaxNode> pass(const TreeBuilderParams&) {
    return nullptr;
  }

 public:
  using BuilderT = std::function<unique_ptr<SyntaxNode>(TreeBuilderParams&)>;

  unique_ptr<Consumer> consumer;
  BuilderT builder;

  Branch(unique_ptr<Consumer> consumer, BuilderT builder = pass)
      : consumer(std::move(consumer)), builder(std::move(builder)) {}
};

struct BranchConsumptionNode final : ConsumptionNode {
  const Branch* branch;

  unique_ptr<SyntaxNode> build_syntax_tree(TreeBuilderParams& params) override {
    return branch->builder(params);
  }
};

class ConcatenatedConsumer final : public Consumer {
  unique_ptr<Consumer> left_;
  unique_ptr<Consumer> right_;

 public:
  ConcatenatedConsumer(unique_ptr<Consumer> left, unique_ptr<Consumer> right)
      : left_(std::move(left)), right_(std::move(right)) {}

  bool consume(
      TokenIteratorT& iterator, const TokenIteratorT& end,
      const GrammarRulesT& rules,
      const unique_ptr<ConsumptionNode>& consumption_node) const override {
    Logger::syntax(LogLevel::DEBUG, "Consuming concatenated 1");

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

class TokenConsumer final : public Consumer {
  TokenType token_;
  string value_;

 public:
  explicit TokenConsumer(TokenType token_type, string value = "")
      : token_(token_type), value_(std::move(value)) {}

  bool consume(
      TokenIteratorT& iterator, const TokenIteratorT& end,
      const GrammarRulesT& rules,
      const unique_ptr<ConsumptionNode>& consumption_node) const override {
    Logger::syntax(LogLevel::DEBUG, "Consuming token: {}",
                   GetTokenDescription({token_, value_}));

    if (iterator == end) {
      Logger::syntax(LogLevel::DEBUG, "Met in reality: end of tokens");
      return false;
    }

    Logger::syntax(LogLevel::DEBUG, "Met in reality: {}",
                   GetTokenDescription(*iterator));

    if (iterator->type != token_) {
      return false;
    }

    if (!value_.empty() && iterator->value != value_) {
      return false;
    }

    auto node = std::make_unique<TokenConsumptionNode>();
    node->token = *iterator;
    consumption_node->children.push_back(std::move(node));

    ++iterator;
    return true;
  }
};

class EmptyConsumer final : public Consumer {
 public:
  bool consume(
      TokenIteratorT& iterator, const TokenIteratorT& end,
      const GrammarRulesT& rules,
      const unique_ptr<ConsumptionNode>& consumption_node) const override {
    Logger::syntax(LogLevel::DEBUG, "Consuming empty");

    return true;
  }
};

class BranchedConsumer final : public Consumer {
  vector<Branch> branches_;

 public:
  BranchedConsumer() = default;

  void add_branch(Branch&& branch) { branches_.push_back(std::move(branch)); }

  bool consume(
      TokenIteratorT& iterator, const TokenIteratorT& end,
      const GrammarRulesT& rules,
      const unique_ptr<ConsumptionNode>& consumption_node) const override {
    Logger::syntax(LogLevel::DEBUG, "Consuming branched");

    unique_ptr<ConsumptionNode> node =
        std::make_unique<BranchConsumptionNode>();

    for (const auto& branch : branches_) {
      node->children.clear();

      if (branch.consumer->consume(iterator, end, rules, node)) {
        static_cast<BranchConsumptionNode*>(node.get())->branch = &branch;
        consumption_node->children.push_back(std::move(node));

        return true;
      }
    }

    return false;
  }
};

class RuleConsumer final : public Consumer {
  RuleIdentifierT rule_;

 public:
  explicit RuleConsumer(RuleIdentifierT rule_name)
      : rule_(std::move(rule_name)) {}

  bool consume(
      TokenIteratorT& iterator, const TokenIteratorT& end,
      const GrammarRulesT& rules,
      const unique_ptr<ConsumptionNode>& consumption_node) const override {
    Logger::syntax(LogLevel::DEBUG, "Consuming rule: {}", rule_);

    return rules.at(rule_).consume(iterator, end, rules, consumption_node);
  }
};

inline auto operator+(unique_ptr<Consumer> left, unique_ptr<Consumer> right) {
  return std::make_unique<ConcatenatedConsumer>(std::move(left),
                                                std::move(right));
}

// idk wtf is happening with reference symbol here but its not me its
// clang-format
inline decltype(auto) operator|=(BranchedConsumer & left, Branch && branch) {
  left.add_branch(std::move(branch));
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
