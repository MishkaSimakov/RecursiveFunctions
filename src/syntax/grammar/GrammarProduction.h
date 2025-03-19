#pragma once

#include <algorithm>
#include <list>
#include <string>
#include <string_view>
#include <variant>

#include "lexis/Token.h"
#include "utils/Hashers.h"
#include "utils/TupleUtils.h"

namespace Syntax {
class NonTerminal {
  size_t id_;

 public:
  explicit NonTerminal(size_t id) : id_(id) {}

  size_t get_id() const { return id_; }

  ssize_t as_number() const { return -static_cast<ssize_t>(id_ + 2); }

  bool operator==(const NonTerminal&) const = default;
};

class Terminal {
  Lexis::TokenType token_;

 public:
  Terminal(Lexis::TokenType token) : token_(token) {}

  Lexis::TokenType get_token() const { return token_; }

  bool operator==(const Terminal&) const = default;
};

class GrammarProductionResult {
 public:
  using PartT = std::variant<Terminal, NonTerminal>;
  static constexpr size_t cTerminalIndex =
      variant_type_index_v<Terminal, PartT>;
  static constexpr size_t cNonTerminalIndex =
      variant_type_index_v<NonTerminal, PartT>;
  static constexpr ssize_t cRuleEndNumber = -1;

 private:
  std::list<PartT> parts_;

 public:
  GrammarProductionResult() = default;
  GrammarProductionResult(Terminal part) : parts_({part}) {}
  GrammarProductionResult(NonTerminal part) : parts_({part}) {}

  const std::list<PartT>& get_parts() const { return parts_; }

  void add_part(const PartT& part) { parts_.emplace_back(part); }

  GrammarProductionResult& operator+=(GrammarProductionResult other) {
    parts_.splice(parts_.end(), std::move(other.parts_));
    return *this;
  }

  struct const_iterator {
   private:
    std::list<PartT>::const_iterator iterator_;

    const_iterator(std::list<PartT>::const_iterator iterator)
        : iterator_(iterator) {}

   public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = void;
    using reference = void;
    using difference_type = size_t;

    const_iterator operator++() {
      ++iterator_;
      return *this;
    }

    const_iterator operator++(int) {
      auto copy = *this;
      ++*this;
      return copy;
    }

    const_iterator operator--() {
      --iterator_;
      return *this;
    }

    bool operator==(const const_iterator&) const = default;

    bool is_terminal() const { return iterator_->index() == cTerminalIndex; }

    Lexis::TokenType access_terminal() const {
      return std::get<cTerminalIndex>(*iterator_).get_token();
    }

    NonTerminal access_nonterminal() const {
      return std::get<cNonTerminalIndex>(*iterator_);
    }

    // cRuleEndNumber = -1 is reserved for rule end so this function doesn't
    // return it
    ssize_t as_number() const {
      if (is_terminal()) {
        return static_cast<size_t>(access_terminal());
      }

      return access_nonterminal().as_number();
    }

    size_t hash() const { return hash_fn(&*iterator_); }

    friend GrammarProductionResult;
  };

  bool is_terminal() const {
    return std::ranges::all_of(parts_, [](const PartT& part) {
      return part.index() == cTerminalIndex;
    });
  }

  bool operator==(const GrammarProductionResult& other) const {
    auto first = cbegin();
    auto second = other.cbegin();

    for (; first != cend() && second != other.cend(); ++first, ++second) {
      if (first.as_number() != second.as_number()) {
        return false;
      }
    }

    return first == cend() && second == other.cend();
  }

  bool empty() const { return parts_.empty(); }

  size_t size() const { return parts_.size(); }

  const_iterator cbegin() const { return const_iterator{parts_.cbegin()}; }
  const_iterator cend() const { return const_iterator{parts_.cend()}; }
};

inline GrammarProductionResult operator+(GrammarProductionResult left,
                                         GrammarProductionResult right) {
  left += std::move(right);
  return left;
}
}  // namespace Syntax

template <>
struct std::hash<Syntax::NonTerminal> {
  size_t operator()(Syntax::NonTerminal value) const noexcept {
    return std::hash<size_t>()(value.get_id());
  }
};

template <>
struct std::hash<Syntax::GrammarProductionResult::const_iterator> {
  size_t operator()(const Syntax::GrammarProductionResult::const_iterator& itr)
      const noexcept {
    return itr.hash();
  }
};
