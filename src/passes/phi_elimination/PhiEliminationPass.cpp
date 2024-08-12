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

    for (auto value : phi_node->values_view()) {
      if (value.is_temporary() && phi_node->return_value != value) {
        can_omit_moves = false;
        break;
      }
    }

    for (auto [parent, value] : phi_node->parents) {
      if (can_omit_moves && value.is_temporary()) {
        continue;
      }

      auto insert_itr = parent->instructions.end();
      if (!parent->instructions.empty() &&
          parent->instructions.back()->is_control_flow_instruction()) {
        insert_itr = std::prev(insert_itr);
      }

      parent->instructions.insert(
          insert_itr,
          std::make_unique<IR::Move>(phi_node->return_value, value));
    }

    itr = instructions.erase(itr);
  }
}
