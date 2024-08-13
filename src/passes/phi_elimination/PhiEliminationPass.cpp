#include "PhiEliminationPass.h"

#include "intermediate_representation/Function.h"
#include "passes/PassManager.h"

void Passes::PhiEliminationPass::process_block(IR::Function& function,
                                               IR::BasicBlock& block) {
  auto& instructions = block.instructions;
  bool was_changed = false;

  for (auto itr = instructions.begin(); itr != instructions.end();) {
    auto* phi_node = dynamic_cast<const IR::Phi*>(itr->get());

    if (phi_node == nullptr) {
      ++itr;
      continue;
    }

    auto phi_temporary = IR::Value(function.get_max_temporary_index() + 1,
                                   IR::ValueType::VIRTUAL_REGISTER);

    for (auto [parent, value] : phi_node->parents) {
      auto insert_itr = parent->instructions.end();
      if (!parent->instructions.empty() &&
          parent->instructions.back()->is_control_flow_instruction()) {
        insert_itr = std::prev(insert_itr);
      }

      parent->instructions.insert(
          insert_itr, std::make_unique<IR::Move>(phi_temporary, value));
    }

    *itr = std::make_unique<IR::Move>(phi_node->return_value, phi_temporary);

    was_changed = true;
  }

  if (was_changed) {
    function.finalize();
    manager_.invalidate();
  }
}
