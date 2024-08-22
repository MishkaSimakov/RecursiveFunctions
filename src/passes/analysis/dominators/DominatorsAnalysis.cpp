#include "DominatorsAnalysis.h"

#include "DynamicBitset.h"

bool Passes::DominatorsAnalysis::dominate(const IR::BasicBlock* first,
                                          const IR::BasicBlock* second) const {
  return blocks_info.at(second).dominators.contains(first);
}

const std::unordered_set<const IR::BasicBlock*>&
Passes::DominatorsAnalysis::dominators(const IR::BasicBlock* block) const {
  return blocks_info.at(block).dominators;
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
  blocks_info.clear();
  loops.clear();

  for (auto& function : program.functions) {
    find_dominators(function);
    find_natural_loops(function);

    join_natural_loops(function);
    calculate_nesting_level(function);
  }
}

void Passes::DominatorsAnalysis::find_dominators(const IR::Function& function) {
  // initialize dominators
  std::unordered_set<const IR::BasicBlock*> all_blocks_set;
  for (auto& block : function.basic_blocks) {
    all_blocks_set.insert(&block);
  }

  for (auto& block : function.basic_blocks) {
    if (&block == function.begin_block) {
      blocks_info[&block].dominators = {&block};
    } else {
      blocks_info[&block].dominators = all_blocks_set;
    }
  }

  // do simple worklist algorithm to find fixed-point dfa solution
  std::unordered_set<const IR::BasicBlock*> worklist;

  // process each block at least once
  auto process_block = [&worklist, this](const IR::BasicBlock* block) {
    worklist.erase(block);

    auto& dominators = blocks_info[block].dominators;

    std::unordered_set<const IR::BasicBlock*> updated;

    if (!block->parents.empty()) {
      // do set intersection
      updated = blocks_info[block->parents.front()].dominators;

      for (auto parent : block->parents) {
        std::erase_if(updated, [this, parent](const IR::BasicBlock* b) {
          return !blocks_info[parent].dominators.contains(b);
        });
      }
    }

    updated.insert(block);

    if (updated == dominators) {
      return;
    }

    dominators = updated;

    for (auto child : block->nonnull_children()) {
      worklist.insert(child);
    }
  };

  function.reversed_postorder_traversal(
      [&worklist, &process_block](const IR::BasicBlock* current) {
        worklist.insert(current);
        process_block(current);
      });

  while (!worklist.empty()) {
    auto current = *worklist.begin();
    process_block(current);
  }
}

void Passes::DominatorsAnalysis::find_natural_loops(
    const IR::Function& function) {
  loops[&function];

  for (auto& block : function.basic_blocks) {
    for (auto child : block.children) {
      if (dominate(child, &block)) {
        Loop loop;

        loop.header = child;
        mark_loop_recursively(loop, &block, child);
        loop.blocks.insert(child);

        // calculating exit blocks
        for (auto loop_block : loop.blocks) {
          for (auto loop_child : loop_block->children) {
            if (loop_child != nullptr && !loop.blocks.contains(loop_child)) {
              loop.exit_blocks.insert(loop_block);
            }
          }
        }

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

void Passes::DominatorsAnalysis::mark_loop_recursively(
    Loop& loop, const IR::BasicBlock* current, const IR::BasicBlock* header) {
  if (current == nullptr || current == header) {
    return;
  }

  if (loop.blocks.contains(current)) {
    return;
  }

  loop.blocks.insert(current);

  for (auto parent : current->parents) {
    mark_loop_recursively(loop, parent, header);
  }
}
