#include "RegisterAllocator.h"

#include "DependenciesGraphBuilder.h"

std::unordered_map<IR::Temporary, ssize_t> IR::RegisterAllocator::get_colouring(
    const TemporaryDependenciesGraph& graph) const {
  // 0, 1, ... - colors, -1 - spill
  using ColouringT = std::pair<double, std::vector<ssize_t>>;
  auto comparator = [](const ColouringT& first, const ColouringT& second) {
    return first.first < second.first;
  };

  std::vector<ColouringT> queue;
  ColouringT top;

  queue.emplace_back(0, std::vector<ssize_t>{});
  while (!queue.empty()) {
    std::ranges::pop_heap(queue, comparator);
    top = std::move(queue.back());

    auto& colouring = top.second;

    if (colouring.size() == graph.temporaries.size()) {
      break;
    }

    queue.pop_back();

    // we should take into account each color and spilling
    for (size_t color = 0; color < kRegistersCount; ++color) {
      double cost = top.first;
      size_t current_node = colouring.size();

      for (size_t j = 0; j < colouring.size(); ++j) {
        // we only care aboud cost if colors are the same
        if (colouring[j] != color) {
          continue;
        }

        double edge_cost = graph.edges[current_node][j];

        if (edge_cost == -TemporaryDependenciesGraph::kInfinity) {
          cost = -TemporaryDependenciesGraph::kInfinity;
          break;
        }

        cost += edge_cost;
      }

      if (cost == -TemporaryDependenciesGraph::kInfinity) {
        continue;
      }

      // add into priority queue
      colouring.push_back(color);
      queue.emplace_back(cost, colouring);
      colouring.pop_back();
    }

    // we should consider spilling
    double cost = top.first + graph.temporaries[top.first].spill_cost;

    colouring.push_back(-1);
    queue.emplace_back(cost, std::move(colouring));
  }

  std::unordered_map<Temporary, ssize_t> result;

  for (size_t i = 0; i < top.second.size(); ++i) {
    result.emplace(graph.temporaries[i].temporary, top.second[i]);
  }

  return result;
}

void IR::RegisterAllocator::apply_to_function(Function& function) {
  DependenciesGraphBuilder builder;

  // create edges between temporaries that are alive simultaneously
  auto graph = builder(function);

  // create edges between temporaries that are in phi node together

  // setup fine for temporaries spilling

  // start Dijkstra on resulting graph colourings (yep, not that fast but finds
  // precise solution)
  auto colouring = get_colouring(graph);

  // apply result to IR (create move, load and store instructions)
}

void IR::RegisterAllocator::apply(Program& program) {
  // every function is processed separately
  for (auto& function : program.functions) {
    apply_to_function(function);
  }
}
