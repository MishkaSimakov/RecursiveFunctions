#ifndef SYNTAXTREEBUILDER_H
#define SYNTAXTREEBUILDER_H

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "lexis/LexicalAnalyzer.h"
#include "log/Logger.h"

using std::string, std::vector, std::cout, std::endl, std::unique_ptr;

enum class SyntaxNodeType { VARIABLE, ASTERISK, FUNCTION, ASSIGNMENT };

/*
some idea for grammar description in program

auto grammar = {
  a: x + y | a + b + c | ...,
  b: x + y | a + c + consume(TokenType::LPAREN) + ...
};
*/

namespace SyntaxConsumers {
class ConcatenatedConsumer;
class CombinedConsumer;
class Consumer;

using RuleIdentifierT = std::size_t;
using GrammarRulesT =
    std::unordered_map<RuleIdentifierT, std::unique_ptr<Consumer>>;

class Consumer {
 protected:
  using TokenIteratorT = vector<Token>::const_iterator;

 public:
  virtual bool consume(TokenIteratorT& iterator, const TokenIteratorT& end,
                       const GrammarRulesT& rules) = 0;

  virtual ~Consumer() = default;
};

class ConcatenatedConsumer : public Consumer {
  unique_ptr<Consumer> left_;
  unique_ptr<Consumer> right_;

 public:
  ConcatenatedConsumer(unique_ptr<Consumer> left, unique_ptr<Consumer> right)
      : left_(std::move(left)), right_(std::move(right)) {}

  bool consume(TokenIteratorT& iterator, const TokenIteratorT& end,
               const GrammarRulesT& rules) override {
    Logger::syntax("Consuming concatenated 1");

    auto copy = iterator;

    if (!left_->consume(iterator, end, rules)) {
      return false;
    }

    if (!right_->consume(iterator, end, rules)) {
      iterator = copy;
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
               const GrammarRulesT& rules) override {
    Logger::syntax("Consuming combined 1");

    if (left_->consume(iterator, end, rules)) {
      return true;
    }

    Logger::syntax("Consuming combined 2");

    if (right_->consume(iterator, end, rules)) {
      return true;
    }

    return false;
  }
};

class TokenConsumer : public Consumer {
  TokenType token_;
  string value_;

 public:
  explicit TokenConsumer(TokenType token_type, string value = "")
      : token_(token_type), value_(std::move(value)) {}

  bool consume(TokenIteratorT& iterator, const TokenIteratorT& end,
               const GrammarRulesT& rules) override {
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

    ++iterator;
    return true;
  }
};

class RuleConsumer : public Consumer {
  RuleIdentifierT rule_name_;

 public:
  explicit RuleConsumer(RuleIdentifierT rule_name)
      : rule_name_(std::move(rule_name)) {}

  bool consume(TokenIteratorT& iterator, const TokenIteratorT& end,
               const GrammarRulesT& rules) override {
    Logger::syntax("Consuming rule:", rule_name_);

    return rules.at(rule_name_)->consume(iterator, end, rules);
  }
};

class EmptyConsumer : public Consumer {
 public:
  bool consume(TokenIteratorT& iterator, const TokenIteratorT& end,
               const GrammarRulesT& rules) override {
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
  left          = std::move(combined);
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

struct SyntaxNode {
  SyntaxNodeType type;
  string value;
  vector<SyntaxNode> children;

  SyntaxNode(const SyntaxNode& other) {
    std::cout << "copied" << std::endl;

    type     = other.type;
    value    = other.value;
    children = other.children;
  }
};

class SyntaxTreeBuilder {
 public:
  static void build(const vector<Token>& program,
                    const SyntaxConsumers::GrammarRulesT& rules,
                    const SyntaxConsumers::RuleIdentifierT& start) {
    auto position = program.begin();
    auto end      = program.end();

    if (rules.at(start)->consume(position, end, rules) && position == end) {
      cout << "Success!" << endl;
      return;
    }

    throw std::runtime_error("No rules matched your program.");
  }
};

#endif  // SYNTAXTREEBUILDER_H
