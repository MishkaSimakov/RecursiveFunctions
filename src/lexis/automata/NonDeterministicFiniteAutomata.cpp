#include "NonDeterministicFiniteAutomata.h"

#include <algorithm>
#include <queue>

#include "FiniteAutomata.h"

struct NonDeterministicFiniteAutomata::FiniteAutomataBuilderVisitor final
    : RegexConstNodeVisitor {
  NonDeterministicFiniteAutomata result;

  void visit(const SymbolRangeNode& node) override {
    auto& start_node = result.get_start_node();
    auto& end_node = result.add_node();

    for (char symbol = node.from; symbol <= node.to; ++symbol) {
      start_node.jumps.emplace(symbol, &end_node);
    }

    end_node.is_final = true;
  }

  void visit(const ConcatenationNode& node) override {
    NonDeterministicFiniteAutomata concatenated;

    node.left->accept(*this);

    std::swap(concatenated, result);

    node.right->accept(*this);

    for (Node& final_node : concatenated.get_final_nodes()) {
      final_node.is_final = false;
      final_node.jumps.emplace(kEmptyJumpSymbol, &result.get_start_node());
    }

    result.nodes_.splice(result.nodes_.begin(), concatenated.nodes_);
  }

  void visit(const OrNode& node) override {
    NonDeterministicFiniteAutomata concatenated;

    node.left->accept(*this);

    std::swap(concatenated, result);

    node.right->accept(*this);

    Node& left_front = concatenated.get_start_node();
    Node& right_front = result.get_start_node();

    Node& front = result.nodes_.emplace_front();
    front.jumps.emplace(kEmptyJumpSymbol, &left_front);
    front.jumps.emplace(kEmptyJumpSymbol, &right_front);

    result.nodes_.splice(result.nodes_.end(), concatenated.nodes_);
  }

  void visit(const StarNode& node) override {
    node.child->accept(*this);

    auto& old_front = result.get_start_node();
    auto& front = result.nodes_.emplace_front();

    front.jumps.emplace(kEmptyJumpSymbol, &old_front);

    for (auto& final_node : result.get_final_nodes()) {
      final_node.is_final = false;
      final_node.jumps.emplace(kEmptyJumpSymbol, &front);
    }

    front.is_final = true;
  }

  void visit(const PlusNode& node) override {
    NonDeterministicFiniteAutomata copy;

    node.child->accept(*this);
    std::swap(result, copy);
    node.child->accept(*this);

    // now we concatenate two copies

    auto& first_part_start = result.get_start_node();
    auto& second_part_start = copy.get_start_node();

    auto& bridge = result.nodes_.emplace_front();
    auto& new_start = result.nodes_.emplace_front();

    new_start.jumps.emplace(kEmptyJumpSymbol, &first_part_start);

    for (auto& final_node : result.get_final_nodes()) {
      final_node.is_final = false;
      final_node.jumps.emplace(kEmptyJumpSymbol, &bridge);
    }

    bridge.is_final = true;

    bridge.jumps.emplace(kEmptyJumpSymbol, &second_part_start);
    for (auto& final_node : copy.get_final_nodes()) {
      final_node.is_final = false;
      final_node.jumps.emplace(kEmptyJumpSymbol, &bridge);
    }

    result.nodes_.splice(result.nodes_.end(), copy.nodes_);
  }
};

void NonDeterministicFiniteAutomata::remove_unreachable_nodes() {
  std::unordered_set<const Node*> visited;
  std::queue<const Node*> queue;

  visited.insert(&nodes_.front());
  queue.push(&nodes_.front());

  while (!queue.empty()) {
    const Node* current = queue.front();
    queue.pop();

    for (const Node* next : current->jumps | std::views::values) {
      auto [itr, was_inserted] = visited.insert(next);

      if (was_inserted) {
        queue.push(next);
      }
    }
  }

  for (auto itr = nodes_.begin(); itr != nodes_.end();) {
    if (visited.contains(&*itr)) {
      ++itr;
    } else {
      itr = nodes_.erase(itr);
    }
  }
}

NonDeterministicFiniteAutomata::NonDeterministicFiniteAutomata(
    const Regex& regex)
    : nodes_([&regex] {
        FiniteAutomataBuilderVisitor visitor;
        regex.get_root().accept(visitor);

        return std::move(visitor.result.nodes_);
      }()) {}

void NonDeterministicFiniteAutomata::remove_empty_jumps() {
  for (auto& node : nodes_) {
    std::unordered_set<Node*> nodes;
    nodes.insert(&node);

    do_empty_jumps(nodes);
    nodes.erase(&node);

    // add appropriate jumps
    std::vector<std::pair<Charset::CharacterT, Node*>> additional_jumps;
    for (const Node* reachable_by_empty_jump : nodes) {
      if (reachable_by_empty_jump->is_final) {
        node.is_final = true;
      }

      for (const auto& [symbol, dest] : reachable_by_empty_jump->jumps) {
        if (symbol != kEmptyJumpSymbol) {
          additional_jumps.emplace_back(symbol, dest);
        }
      }
    }

    for (auto& [symbol, dest] : additional_jumps) {
      node.jumps.emplace(symbol, dest);
    }

    // remove all empty jumps from node
    node.jumps.erase(kEmptyJumpSymbol);
  }

  remove_unreachable_nodes();
}
