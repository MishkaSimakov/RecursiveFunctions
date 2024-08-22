#include "LoopRotationPass.h"

bool Passes::LoopRotationPass::apply(IR::Function& function) {
  bool was_changed = false;

  auto& dominators_analysis = manager_.get_analysis<DominatorsAnalysis>();

  for (const auto& loop : dominators_analysis.get_loops(function)) {
    auto loop_copy = loop;
    bool was_rotated = rotate_loop(function, loop_copy);

    if (was_rotated) {
      was_changed = true;

      dominators_analysis.change_loop_header(function, loop.header,
                                             loop_copy.header);
      function.finalize();
      dominators_analysis.update_loops_information(function);
    }
  }

  return was_changed;
}

bool Passes::LoopRotationPass::rotate_loop(IR::Function& function,
                                           DominatorsAnalysis::Loop& loop) {
  // loop must have only one exit in header block
  if (loop.exit_blocks.size() != 1 || !loop.exit_blocks.contains(loop.header)) {
    return false;
  }

  // header must not be connected to itself
  for (auto parent : loop.header->parents) {
    if (parent == loop.header) {
      return false;
    }
  }

  // one of header's children exits loop and the other goes into loop
  IR::BasicBlock* loop_new_header = nullptr;
  IR::BasicBlock* loop_outer_block = nullptr;
  for (auto child : loop.header->children) {
    if (loop.blocks.contains(child)) {
      loop_new_header = child;
    } else {
      loop_outer_block = child;
    }
  }

  if (loop_new_header == nullptr || loop_outer_block == nullptr) {
    throw std::runtime_error("Something strange in LoopRotationPass.");
  }

  // for each assigned value in loop header we create many copies. One would
  // be assigned inside loop and other outside.
  std::unordered_map<IR::Value,
                     std::vector<std::pair<IR::BasicBlock*, IR::Value>>>
      header_values_replacements;

  // we duplicate header block and connect copies to his parents
  // also we remove phi nodes in header because now it only contains one
  // parent
  for (auto parent : loop.header->parents) {
    auto copy = function.add_block(loop.header->copy_instructions());
    copy->children = loop.header->children;

    if (loop.blocks.contains(parent)) {
      loop.blocks.insert(copy);
    }

    std::unordered_map<IR::Value, IR::Value> replacements;

    for (auto& instruction : loop.header->instructions) {
      if (instruction->has_return_value()) {
        auto return_value = instruction->get_return_value();
        auto copy_value = function.allocate_vreg();

        replacements.emplace(return_value, copy_value);
        header_values_replacements[return_value].emplace_back(copy, copy_value);
      }
    }

    for (auto& instruction : copy->instructions) {
      if (instruction->is_of_type<IR::Phi>()) {
        auto& phi = static_cast<const IR::Phi&>(*instruction);

        for (auto [phi_parent, phi_value] : phi.parents) {
          if (phi_parent == parent) {
            auto move = std::make_unique<IR::Move>(phi.return_value, phi_value);

            if (replacements.contains(move->return_value)) {
              move->return_value = replacements[move->return_value];
            }

            instruction = std::move(move);
            break;
          }
        }
      } else {
        instruction->replace_values(replacements);
      }
    }

    for (auto& child : parent->children) {
      if (child == loop.header) {
        child = copy;
      }
    }
  }

  // we must insert phi instruction in the first block outside of loop
  std::unordered_map<IR::Value, IR::Value> loop_header_values_replacements;
  for (auto& [value, replacements] : header_values_replacements) {
    auto new_value = function.allocate_vreg();

    auto phi = std::make_unique<IR::Phi>();

    phi->return_value = new_value;
    phi->parents = replacements;

    loop_header_values_replacements.emplace(value, new_value);

    loop_outer_block->instructions.push_front(std::move(phi));
  }

  for (auto& block : function.basic_blocks) {
    if (loop.blocks.contains(&block)) {
      continue;
    }

    for (auto& instruction : block.instructions) {
      instruction->replace_values(loop_header_values_replacements);
    }
  }

  for (auto& [value, replacements] : header_values_replacements) {
    auto phi = std::make_unique<IR::Phi>();

    phi->return_value = value;
    phi->parents = std::move(replacements);

    loop_new_header->instructions.push_front(std::move(phi));
  }

  std::erase_if(function.basic_blocks, [&loop](const IR::BasicBlock& block) {
    return &block == loop.header;
  });

  loop.header = loop_new_header;

  return true;
}
