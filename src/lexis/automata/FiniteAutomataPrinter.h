#pragma once

#include <iostream>

#include "FiniteAutomata.h"

template <typename T>
  requires std::same_as<T, FiniteAutomata> ||
           std::same_as<T, NonDeterministicFiniteAutomata>
class FiniteAutomataPrinter {
  const T& automata_;

  // for deterministic
  static void print_automata_edges(const FiniteAutomata& automata,
                                   std::ostream& os) {
    for (size_t i = 0; i < automata.nodes.size(); ++i) {
      const auto& node = automata.nodes[i];

      for (size_t j = 0; j < node.jumps.size(); ++j) {
        if (node.jumps[j] == -1) {
          continue;
        }

        os << fmt::format("{} -> {} [label = \"{}\"];", i, node.jumps[j],
                          static_cast<char>(j))
           << std::endl;
      }
    }
  }

  static std::vector<size_t> get_automata_final_nodes(
      const FiniteAutomata& automata) {
    std::vector<size_t> final_nodes_indices;
    for (size_t i = 0; i < automata.nodes.size(); ++i) {
      if (automata.nodes[i].is_final) {
        final_nodes_indices.push_back(i);
      }
    }

    return final_nodes_indices;
  }

  // for non-deterministic
  static void print_automata_edges(
      const NonDeterministicFiniteAutomata& automata, std::ostream& os) {
    std::unordered_map<const NonDeterministicFiniteAutomata::Node*, size_t>
        indices;
    size_t current_index = 0;
    for (const auto& node : automata.get_nodes()) {
      indices.emplace(&node, current_index);
      ++current_index;
    }

    for (const auto& node : automata.get_nodes()) {
      size_t index = indices[&node];

      for (auto [letter, next] : node.jumps) {
        size_t next_index = indices[next];

        os << fmt::format("{} -> {} [label = \"{}\"];", index, next_index,
                          letter == NonDeterministicFiniteAutomata::kEmptyJumpSymbol
                              ? "ε"
                              : std::string(1, letter))
           << std::endl;
      }
    }
  }

  static std::vector<size_t> get_automata_final_nodes(
      const NonDeterministicFiniteAutomata& automata) {
    std::vector<size_t> final_nodes_indices;
    size_t current_index = 0;
    for (const auto& node : automata.get_nodes()) {
      if (node.is_final) {
        final_nodes_indices.push_back(current_index);
      }

      ++current_index;
    }

    return final_nodes_indices;
  }

 public:
  explicit FiniteAutomataPrinter(const T& automata) : automata_(automata) {}

  void print(std::ostream& os) const {
    auto println = [&os](std::string_view string) { os << string << "\n"; };

    println("digraph finite_state_machine {");
    println("fontname=\"Helvetica,Arial,sans-serif\"");
    println("node [fontname=\"Helvetica,Arial,sans-serif\"]");
    println("edge [fontname=\"Helvetica,Arial,sans-serif\"]");
    println("rankdir=LR;");

    auto final_indices = get_automata_final_nodes(automata_);
    if (!final_indices.empty()) {
      os << fmt::format("node [shape = doublecircle]; {};",
                        fmt::join(final_indices, ", "))
         << std::endl;
    }

    println("node [shape = circle];");

    print_automata_edges(automata_, os);

    println("}");
  }
};

template <typename T>
std::ostream& operator<<(std::ostream& os,
                         const FiniteAutomataPrinter<T>& printer) {
  printer.print(os);
  return os;
}
