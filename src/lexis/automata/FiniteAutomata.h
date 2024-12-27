#pragma once
#include <array>
#include <unordered_set>

#include "lexis/Charset.h"
#include "NonDeterministicFiniteAutomata.h"
#include "lexis/regex/CustomRegex.h"

class FiniteAutomata {
 public:
  struct Node {
    bool is_final{false};

    // -1 - no jump
    std::array<ssize_t, Charset::kCharactersCount> jumps{};
  };

  std::deque<Node> nodes;

  FiniteAutomata();

  explicit FiniteAutomata(const Regex& regex)
      : FiniteAutomata(NonDeterministicFiniteAutomata(regex)) {}
  explicit FiniteAutomata(
      const NonDeterministicFiniteAutomata& finite_automata);

  void remove_dead_ends();

  FiniteAutomata get_minimal() const;
};
