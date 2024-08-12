#include "DominatorsAnalysis.h"

#include "DynamicBitset.h"

bool Passes::DominatorsAnalysis::dominate(size_t function_index,
                                          const IR::BasicBlock* first,
                                          const IR::BasicBlock* second) const {
  return dom_info.at(function_index).at(second).contains(first);
}

void Passes::DominatorsAnalysis::perform_analysis(const IR::Program& program) {
  dom_info.resize(program.functions.size());

  for (size_t i = 0; i < program.functions.size(); ++i) {
    dom_info[i] = get_dom_for_function(program.functions[i]);
  }
}

void Passes::DominatorsAnalysis::process_function(const IR::Function& function) {
  DominatorsTreeInfo info;

  std::unordered_map<const IR::BasicBlock*, DynamicBitset> dominators;
  size_t blocks_count = function.basic_blocks.size();

  for (size_t i = 0; i < blocks_count; ++i) {
    DynamicBitset bitset(blocks_count);
    bitset[i] = true;

    dominators.emplace(&function.basic_blocks[i], std::move(bitset));
  }

  function.postorder_traversal(
      [&dominators, blocks_count](const IR::BasicBlock* block) {
        DynamicBitset doms(blocks_count);
        doms.flip();

        for (auto& parent : block->parents) {
          doms &= dominators[parent];
        }

        dominators[block] |= doms;
      });

  for (auto& [block, doms] : dominators) {
    for (size_t i = 0; i < blocks_count; ++i) {
      if (doms[i]) {
        info[block].insert(&function.basic_blocks[i]);
      }
    }
  }

  return info;
}
