#include "RegisterAllocator.h"

#include <numeric>

#include "DependenciesGraphBuilder.h"
#include "intermediate_representation/Function.h"

std::unordered_map<IR::Temporary, ssize_t> IR::RegisterAllocator::get_colouring(
    const TemporaryDependenciesGraph& graph) const {
  // 0, 1, ... - colors, -1 - spill
  using ColouringT = std::pair<double, std::vector<ssize_t>>;
  auto comparator = [](const ColouringT& first, const ColouringT& second) {
    return first.first < second.first;
  };

  // we should sort temporaries by their desired color cost
  // here we store permutation
  std::vector<size_t> permutation(graph.temporaries.size());
  std::iota(permutation.begin(), permutation.end(), 0);

  std::ranges::sort(
      permutation, [&temp = graph.temporaries](size_t a, size_t b) {
        return temp[a].desired_color.second < temp[b].desired_color.second;
      });

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

    size_t current_node = permutation[colouring.size()];

    // we should take into account each color and spilling
    for (size_t color = 0; color <= kRegistersCount; ++color) {
      double cost = top.first;
      bool is_color_required = false;
      size_t actual_color;

      if (color == 0) {
        auto [desired_color, additional_cost] =
            graph.temporaries[current_node].desired_color;
        if (additional_cost == 0) {
          continue;
        }

        color = desired_color;
        cost += additional_cost;

        if (additional_cost == TemporaryDependenciesGraph::kInfinity) {
          is_color_required = true;
        }
      } else {
        actual_color = color - 1;
      }

      for (size_t j = 0; j < colouring.size(); ++j) {
        // we only care aboud cost if colors are the same
        if (colouring[j] != color) {
          continue;
        }

        double edge_cost = graph.edges[current_node][permutation[j]];

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
      std::ranges::push_heap(queue, comparator);
      colouring.pop_back();

      if (color == 0 && is_color_required) {
        break;
      }
    }

    // we should consider spilling
    double cost = top.first + graph.temporaries[current_node].spill_cost;

    colouring.push_back(-1);
    queue.emplace_back(cost, std::move(colouring));
    std::ranges::push_heap(queue, comparator);
  }

  std::unordered_map<Temporary, ssize_t> result;

  for (size_t i = 0; i < top.second.size(); ++i) {
    result.emplace(graph.temporaries[i].temporary, top.second[i]);
  }

  return result;
}

void IR::RegisterAllocator::apply_to_function(Function& function) {
  std::cout << "Processing " << function.name << std::endl;

  DependenciesGraphBuilder builder;

  // create edges between temporaries that are alive simultaneously
  auto graph = builder(function);

  // create edges between temporaries that are in phi node together
  add_edges_for_phi(function, graph);

  // setup fine for temporaries spilling
  set_spill_fine(function, graph);

  // start Dijkstra on resulting graph colourings (yep, not that fast but finds
  // precise solution)
  auto colouring = get_colouring(graph);

  // remove phi node (now we can forget about ssa)
  remove_phi_nodes(function, colouring);

  // just print colouring for now
  for (auto [temp, color] : colouring) {
    if (color == -1) {
      fmt::println("{} -> spill", temp);
    } else {
      fmt::println("{} -> w{}", temp, color);
    }
  }
}

void IR::RegisterAllocator::add_edges_for_phi(
    const Function& function, TemporaryDependenciesGraph& graph) {
  for (auto& block : function.basic_blocks) {
    for (auto& instruction : block.instructions) {
      auto* phi_node = dynamic_cast<const Phi*>(instruction.get());

      if (phi_node == nullptr) {
        continue;
      }

      auto temporary_arguments = phi_node->get_temporaries_in_arguments();
      for (size_t i = 0; i < temporary_arguments.size(); ++i) {
        for (size_t j = 0; j < i; ++j) {
          graph.add_dependency(temporary_arguments[i], temporary_arguments[j],
                               kPhiNodeWeight);
        }
      }
    }
  }
}

void IR::RegisterAllocator::set_spill_fine(const Function& function,
                                           TemporaryDependenciesGraph& graph) {
  // TODO: this must depend on variable position
  for (auto& temp : graph.temporaries) {
    temp.spill_cost = kSpillCost;
  }
}

void IR::RegisterAllocator::remove_phi_nodes(
    Function& function, const std::unordered_map<Temporary, ssize_t>&) {
  for (auto& block : function.basic_blocks) {
    auto& instructions = block.instructions;
    for (auto itr = instructions.begin(); itr != instructions.end();) {
      auto* phi_node = dynamic_cast<const Phi*>(itr->get());

      if (phi_node == nullptr) {
        ++itr;
        continue;
      }

      bool can_omit_moves = true;

      for (auto value : phi_node->values | std::views::values) {
        if (value.is_temporary() &&
            phi_node->result_destination != Temporary{value.index()}) {
          can_omit_moves = false;
          break;
        }
      }

      for (auto [block, value] : phi_node->values) {
        if (can_omit_moves && value.is_temporary()) {
          continue;
        }

        auto move_instruction =
            std::make_unique<Move>(phi_node->result_destination, value);
        block->instructions.push_back(std::move(move_instruction));
      }

      itr = instructions.erase(itr);
    }
  }
}

void IR::RegisterAllocator::apply(Program& program) {
  // every function is processed separately
  for (auto& function : program.functions) {
    apply_to_function(function);
  }
}
