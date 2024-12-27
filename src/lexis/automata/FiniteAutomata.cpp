#include "FiniteAutomata.h"

#include <queue>

FiniteAutomata::FiniteAutomata() : nodes(1) {
  nodes.front().jumps.fill(-1);
}

void FiniteAutomata::remove_dead_ends() {
  std::vector is_dead_end(nodes.size(), true);
  std::vector<size_t> queue;

  for (size_t i = 0; i < nodes.size(); ++i) {
    if (nodes[i].is_final) {
      is_dead_end[i] = false;
      queue.push_back(i);
    }
  }

  while (!queue.empty()) {
    size_t current = queue.back();
    queue.pop_back();

    for (size_t i = 0; i < nodes.size(); ++i) {
      if (!is_dead_end[i]) {
        break;
      }

      for (size_t symbol = 0; symbol < Charset::kCharactersCount; ++symbol) {
        if (nodes[i].jumps[symbol] == current) {
          queue.push_back(i);
          is_dead_end[i] = false;
        }
      }
    }
  }

  std::vector<ssize_t> nodes_mapping(nodes.size());
  std::deque<Node> new_nodes;

  for (size_t i = 0; i < nodes.size(); ++i) {
    if (is_dead_end[i]) {
      nodes_mapping[i] = -1;
      continue;
    }

    nodes_mapping[i] = static_cast<ssize_t>(new_nodes.size());
    new_nodes.emplace_back(nodes[i]);
  }

  for (Node& node : new_nodes) {
    for (size_t symbol = 0; symbol < Charset::kCharactersCount; ++symbol) {
      node.jumps[symbol] = nodes_mapping[node.jumps[symbol]];
    }
  }

  nodes = std::move(new_nodes);
}
