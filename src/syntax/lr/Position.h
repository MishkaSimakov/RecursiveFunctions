#pragma once
#include "syntax/grammar/GrammarProduction.h"

namespace Syntax {
struct Position {
  NonTerminal from;
  const std::pair<GrammarProductionResult, size_t>& production;
  GrammarProductionResult::const_iterator iterator;

  Position(NonTerminal from,
           const std::pair<GrammarProductionResult, size_t>& production)
      : from(from),
        production(production),
        iterator(production.first.cbegin()) {}

  bool operator==(const Position& other) const {
    // iterator contains pointer to list element in it,
    // this pointer is unique for different productions
    return iterator == other.iterator;
  }

  GrammarProductionResult::const_iterator end_iterator() const {
    return production.first.cend();
  }

  std::string to_string() const {
    std::string result;

    result += std::to_string(from.get_id()) + " -> ";

    for (auto itr = production.first.cbegin(); itr != production.first.cend();
         ++itr) {
      if (itr == iterator) {
        result += '.';
      }

      if (itr.is_terminal()) {
        result += "(" + std::string(itr.access_terminal().to_string()) + ")";
      } else {
        result += std::to_string(itr.access_nonterminal().get_id());
      }
    }

    if (iterator == production.first.cend()) {
      result += '.';
    }

    return result;
  }
};
}  // namespace Syntax

template <>
struct std::hash<Syntax::Position> {
  size_t operator()(const Syntax::Position& position) const noexcept {
    return std::hash<Syntax::GrammarProductionResult::const_iterator>()(
        position.iterator);
  }
};
