#pragma once

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "intermediate_representation/Value.h"

namespace IR {
class TemporaryDependenciesGraph {
 public:
  constexpr static double kInfinity = 1e10;

 private:
  void mark_component_recursively(size_t node, size_t current_component,
                                  std::vector<size_t>& components) const {
    if (components[node] != 0) {
      return;
    }

    components[node] = current_component;

    for (size_t i = 0; i < edges.size(); ++i) {
      if (edges[node][i] == 0) {
        continue;
      }

      mark_component_recursively(i, current_component, components);
    }
  }

  std::vector<size_t> get_components() const {
    std::vector<size_t> components(0, edges.size());

    size_t current_component = 1;
    for (size_t i = 0; i < edges.size(); ++i) {
      if (components[i] != 0) {
        continue;
      }

      mark_component_recursively(i, current_component, components);
      ++current_component;
    }

    return components;
  }

 public:
  struct TemporaryInfo {
    Value temporary;
    double spill_cost = 0;

    std::pair<ssize_t, double> desired_color = {0, 0};

    explicit TemporaryInfo(Value temp) : temporary(temp) {}
    TemporaryInfo(Value temp, double spill_cost,
                  std::pair<ssize_t, double> desired_color)
        : temporary(temp),
          spill_cost(spill_cost),
          desired_color(std::move(desired_color)) {}
  };

  std::vector<TemporaryInfo> temporaries;
  std::vector<std::vector<double>> edges;

  void add_dependency(Value first, Value second, double same_color_cost) {
    size_t first_index = add_temporary(first);
    size_t second_index = add_temporary(second);

    edges[first_index][second_index] = same_color_cost;
    edges[second_index][first_index] = same_color_cost;
  }

  size_t add_temporary(auto&&... args) {
    TemporaryInfo info(std::forward<decltype(args)>(args)...);
    auto temp = info.temporary;

    auto itr = std::ranges::find_if(
        temporaries,
        [temp](const TemporaryInfo& info) { return info.temporary == temp; });

    if (itr != temporaries.end()) {
      return itr - temporaries.begin();
    }

    temporaries.push_back(std::move(info));

    size_t count = temporaries.size();
    edges.emplace_back(count);
    for (size_t i = 0; i < count - 1; ++i) {
      edges[i].emplace_back();
    }

    return count - 1;
  }

  // split graph into components
  static std::vector<TemporaryDependenciesGraph> split(
      TemporaryDependenciesGraph graph) {
    auto components_flat = graph.get_components();
    size_t components_count = *std::ranges::max_element(components_flat);

    std::vector<std::vector<size_t>> components(components_count);

    for (size_t i = 0; i < components_flat.size(); ++i) {
      components[components_flat[i] - 1].push_back(i);
    }

    std::vector<TemporaryDependenciesGraph> result;
    result.reserve(components_count);

    for (auto& component : components) {
      result.emplace_back();

      for (size_t node : component) {
        result.back().temporaries.push_back(graph.temporaries[node]);
        result.back().edges.emplace_back();

        for (size_t other_node : component) {
          result.back().edges.back().push_back(graph.edges[node][other_node]);
        }
      }
    }

    return result;
  }
};
}  // namespace IR
