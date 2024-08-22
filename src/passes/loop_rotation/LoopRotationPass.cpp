#include "LoopRotationPass.h"

#include "passes/analysis/dominators/DominatorsAnalysis.h"

bool Passes::LoopRotationPass::apply(IR::Function& function) {
  const auto& loops =
      manager_.get_analysis<DominatorsAnalysis>().get_loops(function);

  // TODO: process more than one loop
  if (loops.empty()) {
    return false;
  }

  const auto& loop = loops.front();

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
  for (auto child : loop.header->children) {
    if (loop.blocks.contains(child)) {
      loop_new_header = child;
      break;
    }
  }

  if (loop_new_header == nullptr) {
    throw std::runtime_error("Something strange in LoopRotationPass.");
  }

  // for each assigned value in loop header we create many copies. One would be
  // assigned inside loop and other outside.
  std::unordered_map<IR::Value,
                     std::vector<std::pair<IR::BasicBlock*, IR::Value>>>
      header_values_replacements;

  // we duplicate header block and connect copies to his parents
  // also we remove phi nodes in header because now it only contains one parent
  for (auto parent : loop.header->parents) {
    auto copy = function.add_block(loop.header->copy_instructions());
    copy->children = loop.header->children;

    std::unordered_map<IR::Value, IR::Value> replacements;

    for (auto& instruction : loop.header->instructions) {
      if (instruction->has_return_value()) {
        auto return_value = instruction->get_return_value();
        auto copy_value = function.allocate_vreg();

        replacements.emplace(return_value, copy_value);
        header_values_replacements[return_value].emplace_back(copy, copy_value);
      }
    }

    for (auto itr = copy->instructions.begin();
         itr != copy->instructions.end();) {
      (*itr)->replace_values(replacements);

      if ((*itr)->is_of_type<IR::Phi>()) {
        auto& phi = static_cast<const IR::Phi&>(**itr);

        for (auto [phi_parent, phi_value] : phi.parents) {
          if (phi_parent == parent) {
            *itr = std::make_unique<IR::Move>(phi.return_value, phi_value);
            break;
          }
        }
      }

      ++itr;
    }

    for (auto& child : parent->children) {
      if (child == loop.header) {
        child = copy;
      }
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

  return true;
}
