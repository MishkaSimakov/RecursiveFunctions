#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <deque>
#include <list>
#include <ranges>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "lexis/Charset.h"
#include "lexis/regex/CustomRegex.h"

class NonDeterministicFiniteAutomata {
  struct FiniteAutomataBuilderVisitor;

 public:
  struct Node {
    bool is_final{false};
    std::unordered_multimap<Charset::CharacterT, Node*> jumps;
  };

 private:
  std::list<Node> nodes_;

  void remove_unreachable_nodes();

 public:
  static constexpr Charset::CharacterT kEmptyJumpSymbol =
      Charset::kReservedCharacter;

  NonDeterministicFiniteAutomata() {
    // emplace start node at position 0
    nodes_.emplace_back();
  }

  explicit NonDeterministicFiniteAutomata(const Regex& regex);

  Node& get_start_node() { return nodes_.front(); }
  Node& add_node() { return nodes_.emplace_back(); }

  auto get_final_nodes() {
    return nodes_ |
           std::views::filter([](const Node& node) { return node.is_final; });
  }

  void remove_empty_jumps();

  size_t size() const { return nodes_.size(); }

  Regex get_regex() const;

  const std::list<Node>& get_nodes() const { return nodes_; }

  // return true if one of resulting nodes is final
  template <typename T>
  static bool do_empty_jumps(T& nodes) {
    std::list<const Node*> unprocessed;
    bool is_final = false;

    auto process_node = [&is_final, &unprocessed, &nodes](const Node* node) {
      if (node->is_final) {
        is_final = true;
      }

      auto [beg, end] = node->jumps.equal_range(kEmptyJumpSymbol);
      for (auto* next : std::ranges::subrange{beg, end} | std::views::values) {
        if (!nodes.contains(next)) {
          nodes.insert(next);
          unprocessed.push_back(next);
        }
      }
    };

    std::copy(nodes.begin(), nodes.end(), std::back_inserter(unprocessed));

    while (!unprocessed.empty()) {
      const Node* current = unprocessed.front();
      unprocessed.pop_front();

      process_node(current);
    }

    return is_final;
  }
};
