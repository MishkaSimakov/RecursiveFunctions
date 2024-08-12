#include "DominatorsAnalysis.h"

#include "DynamicBitset.h"

bool Passes::DominatorsAnalysis::dominate(const IR::BasicBlock* first,
                                          const IR::BasicBlock* second) const {
  return blocks_info.at(second).dominators.contains(first);
}

size_t Passes::DominatorsAnalysis::nesting_level(
    const IR::BasicBlock* block) const {
  return blocks_info.at(block).nesting_level;
}

const std::vector<Passes::DominatorsAnalysis::Loop>&
Passes::DominatorsAnalysis::get_loops(const IR::Function& function) const {
  return loops.at(&function);
}

void Passes::DominatorsAnalysis::perform_analysis(const IR::Program& program) {
  for (auto& function : program.functions) {
    find_dominators(function);
    find_natural_loops(function);

    join_natural_loops(function);
    calculate_nesting_level(function);
  }
}

void Passes::DominatorsAnalysis::find_dominators(const IR::Function& function) {
  std::unordered_map<const IR::BasicBlock*, DynamicBitset> temp;

  function.postorder_traversal([this](const IR::BasicBlock* block) {
    auto& dominators = blocks_info[block].dominators;

    if (block->parents.empty()) {
      dominators.insert(block);
      return;
    }

    dominators = blocks_info.at(block->parents.front()).dominators;

    for (auto parent : block->parents | std::views::drop(1)) {
      std::erase_if(dominators, [this, parent](const IR::BasicBlock* b) {
        return !blocks_info.at(parent).dominators.contains(b);
      });
    }

    dominators.insert(block);
  });
}

void Passes::DominatorsAnalysis::find_natural_loops(
    const IR::Function& function) {
  loops[&function];

  for (auto& block : function.basic_blocks) {
    for (auto child : block.children) {
      if (dominate(child, &block)) {
        Loop loop;
        loop.header = child;
        std::unordered_map<const IR::BasicBlock*, bool> visited;
        mark_loop_recursively(loop, child, child, &block, visited);

        loops[&function].push_back(std::move(loop));
      }
    }
  }
}

void Passes::DominatorsAnalysis::join_natural_loops(const IR::Function&) {
  // join natural loops with same header
}

void Passes::DominatorsAnalysis::calculate_nesting_level(
    const IR::Function& function) {
  auto& function_loops = loops.at(&function);

  for (auto& loop : function_loops) {
    for (auto block : loop.blocks) {
      ++blocks_info.at(block).nesting_level;
    }
  }
}

bool Passes::DominatorsAnalysis::mark_loop_recursively(
    Loop& loop, const IR::BasicBlock* current, const IR::BasicBlock* header,
    const IR::BasicBlock* end,
    std::unordered_map<const IR::BasicBlock*, bool>& visited) {
  if (current == header || current == nullptr) {
    return false;
  }

  if (current == end) {
    loop.blocks.insert(current);
    return true;
  }

  if (visited.contains(current)) {
    return visited[current];
  }

  bool has_path = false;
  for (auto child : current->children) {
    if (mark_loop_recursively(loop, child, header, end, visited)) {
      has_path = true;
    }
  }

  if (has_path) {
    loop.blocks.insert(current);
  }

  visited[current] = has_path;
  return has_path;
}
