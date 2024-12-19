#pragma once
#include <bitset>

#include "lexis/LexicalAnalyzer.h"

namespace Syntax {
class TokensBitset {
  constexpr static size_t cSize = Lexing::TokenType::count;

  std::bitset<cSize> storage_;

 public:
  static TokensBitset only_end() {
    TokensBitset result;
    result.add(Lexing::TokenType::END);
    return result;
  }

  void add(const TokensBitset& other) { storage_ |= other.storage_; }

  void add(Lexing::TokenType token) {
    storage_.set(static_cast<size_t>(token));
  }

  void remove(Lexing::TokenType token) {
    storage_.set(static_cast<size_t>(token), false);
  }

  void remove(const TokensBitset& other) { storage_ &= ~other.storage_; }

  bool empty() const { return storage_.none(); }

  bool contains(Lexing::TokenType token) const {
    return storage_.test(static_cast<size_t>(token));
  }

  bool operator==(const TokensBitset&) const = default;

  size_t hash() const noexcept { return hash_fn(storage_); }
};
}  // namespace Syntax

template <>
struct std::hash<Syntax::TokensBitset> {
  size_t operator()(const Syntax::TokensBitset& value) const noexcept {
    return value.hash();
  }
};
