#include "Disentangler.h"

#include <algorithm>

void Passes::Disentangler::disentangle_chains(
    std::vector<std::pair<IR::Value, IR::Value>>& moves,
    std::deque<PermutationNode>& permutation_nodes) {
  for (auto& node : permutation_nodes) {
    if (node.next != nullptr) {
      continue;
    }

    PermutationNode* current = &node;

    while (current->prev != nullptr) {
      moves.emplace_back(current->prev->value, current->value);
      current->value = current->prev->value;

      auto* next_current = current->prev;
      current->prev->next = nullptr;
      current->prev = nullptr;
      current = next_current;
    }
  }
}

std::vector<std::pair<IR::Value, IR::Value>> Passes::Disentangler::disentangle(
    const std::vector<std::pair<IR::Value, IR::Value>>& knot,
    IR::Value temporary) {
  std::vector<std::pair<IR::Value, IR::Value>> moves;

  std::deque<PermutationNode> permutation_nodes;

  // // build permutation graph
  // for (size_t i = 0; i < knot.size(); ++i) {
  //   IR::Value& argument = call->arguments[i];
  //
  //   if (argument.type == IR::ValueType::BASIC_REGISTER &&
  //       argument.value < arguments_count) {
  //     permutation[argument.value].value = &argument;
  //     permutation[argument.value].next = &permutation[i];
  //   } else {
  //     permutation.emplace_back(&argument, &permutation[i], nullptr);
  //   }
  // }

  auto get_node = [&permutation_nodes](IR::Value value) -> size_t {
    auto itr = std::ranges::find_if(
        permutation_nodes,
        [value](const PermutationNode& node) { return node.value == value; });

    if (itr != permutation_nodes.end()) {
      return itr - permutation_nodes.begin();
    }

    permutation_nodes.emplace_back(value);
    return permutation_nodes.size() - 1;
  };

  for (auto [from, to] : knot) {
    if (from == to) {
      continue;
    }

    auto from_index = get_node(from);
    auto to_index = get_node(to);

    permutation_nodes[from_index].next = &permutation_nodes[to_index];
    permutation_nodes[to_index].prev = &permutation_nodes[from_index];
  }

  // first we find and process all chains
  disentangle_chains(moves, permutation_nodes);

  // then we move nodes from loop to temporary and repeat
  bool has_loops;
  do {
    has_loops = false;

    for (auto& node : permutation_nodes) {
      if (node.next == nullptr) {
        continue;
      }

      has_loops = true;

      auto temporary_node_index = get_node(temporary);
      auto& temporary_node = permutation_nodes[temporary_node_index];

      moves.emplace_back(node.value, temporary);
      temporary_node.next = node.next;
      node.next->prev = &temporary_node;
      node.next = nullptr;

      disentangle_chains(moves, permutation_nodes);

      break;
    }
  } while (has_loops);

  return moves;
}
