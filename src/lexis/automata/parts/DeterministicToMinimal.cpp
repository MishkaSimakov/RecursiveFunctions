#include "lexis/automata/FiniteAutomata.h"

#include <queue>

FiniteAutomata FiniteAutomata::get_minimal() const {
  size_t size = nodes.size();

  std::vector<std::vector<std::pair<size_t, size_t>>> back_edges(size);

  for (size_t symbol = 0; symbol < Charset::kCharactersCount; ++symbol) {
    for (size_t i = 0; i < size; ++i) {
      back_edges[nodes[i].jumps[symbol]].emplace_back(symbol, i);
    }
  }

  std::vector equivalence(size, std::vector(size, false));
  std::queue<std::pair<size_t, size_t>> queue;

  for (size_t i = 0; i < size; ++i) {
    for (size_t j = 0; j < i; ++j) {
      if (nodes[i].is_final != nodes[j].is_final) {
        equivalence[i][j] = true;
        equivalence[j][i] = true;

        queue.emplace(i, j);
      }
    }
  }

  auto symbol_proj = [](const std::pair<size_t, size_t>& pair) {
    return pair.first;
  };

  while (!queue.empty()) {
    auto [i, j] = queue.front();
    queue.pop();

    for (size_t symbol = 0; symbol < Charset::kCharactersCount; ++symbol) {
      auto i_range =
          std::ranges::equal_range(back_edges[i], symbol, {}, symbol_proj);

      auto j_range =
          std::ranges::equal_range(back_edges[j], symbol, {}, symbol_proj);

      for (auto i_parent : i_range | std::views::values) {
        for (auto j_parent : j_range | std::views::values) {
          if (!equivalence[i_parent][j_parent]) {
            equivalence[i_parent][j_parent] = true;
            equivalence[j_parent][i_parent] = true;

            queue.emplace(i_parent, j_parent);
          }
        }
      }
    }
  }

  //
  std::vector<size_t> equivalence_classes_representatives;
  std::vector<size_t> equivalence_classes(size);

  for (size_t i = 0; i < size; ++i) {
    int equivalence_class = -1;

    for (size_t j = 0; j < equivalence_classes_representatives.size(); ++j) {
      if (!equivalence[i][equivalence_classes_representatives[j]]) {
        equivalence_class = static_cast<int>(j);
        break;
      }
    }

    if (equivalence_class == -1) {
      equivalence_class =
          static_cast<int>(equivalence_classes_representatives.size());
      equivalence_classes_representatives.push_back(i);
    }

    equivalence_classes[i] = equivalence_class;
  }

  //
  FiniteAutomata result;
  result.nodes.resize(equivalence_classes_representatives.size());

  for (size_t i = 0; i < equivalence_classes_representatives.size(); ++i) {
    const auto& repr = nodes[equivalence_classes_representatives[i]];

    result.nodes[i].is_final = repr.is_final;
    for (size_t j = 0; j < Charset::kCharactersCount; ++j) {
      result.nodes[i].jumps[j] = equivalence_classes[repr.jumps[j]];
    }
  }

  return result;
}
