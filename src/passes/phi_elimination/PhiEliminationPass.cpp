#include "PhiEliminationPass.h"

void Passes::PhiEliminationPass::process_block(IR::Function& function,
                                               IR::BasicBlock& block) {
  auto& instructions = block.instructions;

  for (auto itr = instructions.begin(); itr != instructions.end();) {
    auto* phi_node = dynamic_cast<const IR::Phi*>(itr->get());

    if (phi_node == nullptr) {
      ++itr;
      continue;
    }

    bool can_omit_moves = true;

    for (auto value : phi_node->values | std::views::values) {
      if (value.is_temporary() &&
          phi_node->result_destination != IR::Temporary{value.index()}) {
        can_omit_moves = false;
        break;
      }
    }

    for (auto [block, value] : phi_node->values) {
      if (can_omit_moves && value.is_temporary()) {
        continue;
      }

      block->instructions.push_back(
          std::make_unique<IR::Move>(phi_node->result_destination, value));
    }

    itr = instructions.erase(itr);
  }
}
