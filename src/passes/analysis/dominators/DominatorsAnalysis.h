#pragma once
#include <vector>

#include "passes/analysis/Analyser.h"

namespace Passes {
class DominatorsAnalysis final : public Analyser {
 public:
  using Analyser::Analyser;

  struct Loop {
    std::unordered_set<const IR::BasicBlock*> blocks;
    std::unordered_set<const IR::BasicBlock*> exit_blocks;

    const IR::BasicBlock* header;
  };

  bool dominate(const IR::BasicBlock*, const IR::BasicBlock*) const;

  const std::unordered_set<const IR::BasicBlock*>& dominators(
      const IR::BasicBlock*) const;

  // Update all information about function, but doesn't invalidate loops
  // iterators
  void update_loops_information(const IR::Function&);

  void change_loop_header(const IR::Function& function,
                          const IR::BasicBlock* old_header,
                          const IR::BasicBlock* new_header);

  size_t nesting_level(const IR::BasicBlock*) const;

  const std::vector<Loop>& get_loops(const IR::Function&) const;

 protected:
  struct BlockInfo {
    std::unordered_set<const IR::BasicBlock*> dominators;

    size_t nesting_level{0};
  };

  std::unordered_map<const IR::BasicBlock*, BlockInfo> blocks_info;
  std::unordered_map<const IR::Function*, std::vector<Loop>> loops;

  void perform_analysis(const IR::Program&) override;

  void find_dominators(const IR::Function&);

  void find_natural_loops(const IR::Function&);

  void join_natural_loops(const IR::Function&);
  void calculate_nesting_level(const IR::Function&);

  static void mark_loop_recursively(Loop& loop, const IR::BasicBlock* current,
                                    const IR::BasicBlock* header);
};
}  // namespace Passes
