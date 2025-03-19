#include "ReplaceBranchWithSelect.h"

#include "passes/PassManager.h"

bool Passes::ReplaceBranchWithSelect::can_apply_to_phi(
    const IR::BasicBlock& block, const IR::Phi& instruction) const {
  if (instruction.parents.size() != 2) {
    return false;
  }

  // only one phi instruction in block
  size_t phi_count = 0;
  for (auto& instr : block.instructions) {
    if (!instr->is_of_type<IR::Phi>()) {
      continue;
    }

    ++phi_count;

    if (phi_count >= 2) {
      return false;
    }
  }

  const IR::BasicBlock* granny = nullptr;

  // there must be diamond-like shape without any loops
  for (auto parent : instruction.parents | std::views::keys) {
    // no loops
    if (parent == &block) {
      return false;
    }

    // only one child and one granny
    if (!parent->has_one_child() || !parent->has_one_parent()) {
      return false;
    }

    // not so many instructions
    if (parent->instructions.size() > 3) {
      return false;
    }

    // same granny for every parent
    if (granny != nullptr && granny != parent->parents.front()) {
      return false;
    }

    granny = parent->parents.front();

    // no loops
    if (granny == &block) {
      return false;
    }

    // no calls to other functions
    for (auto& parent_instr : parent->instructions) {
      if (parent_instr->is_of_type<IR::FunctionCall>()) {
        return false;
      }
    }
  }

  return true;
}

bool Passes::ReplaceBranchWithSelect::apply(IR::Function& function) {
  bool was_changed = false;

  for (auto& block : function.basic_blocks) {
    for (auto itr = block.instructions.begin(); itr != block.instructions.end();
         ++itr) {
      auto& instr = *itr;
      // we search for phi instruction
      if (!instr->is_of_type<IR::Phi>()) {
        continue;
      }

      auto& phi = static_cast<const IR::Phi&>(*instr);

      if (!can_apply_to_phi(block, phi)) {
        break;
      }

      was_changed = true;

      auto granny = phi.parents.front().first->parents.front();

      // remove branch instruction
      auto condition_value =
          static_cast<const IR::Branch*>(granny->instructions.back().get())
              ->arguments[0];
      granny->instructions.pop_back();

      // we should remove jumps from parents and move parents code into granny
      for (auto [parent, value] : phi.parents) {
        parent->instructions.pop_back();
        parent->children = {nullptr, nullptr};

        granny->instructions.splice(granny->instructions.end(),
                                    parent->instructions);
      }

      granny->children = {&block, nullptr};

      // replace phi with select
      instr = std::make_unique<IR::Select>(phi.return_value, condition_value,
                                           phi.parents[0].second,
                                           phi.parents[1].second);

      function.finalize();
    }
  }

  if (was_changed) {
  function.simplify_blocks();
  }

  return was_changed;
}
