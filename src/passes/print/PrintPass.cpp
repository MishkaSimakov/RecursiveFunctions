#include "PrintPass.h"

#include <algorithm>
#include <ranges>

#include "intermediate_representation/Function.h"
#include "passes/PassManager.h"
#include "passes/analysis/liveness/LivenessAnalysis.h"

Passes::PrintPass::PrintPass(PassManager& manager, std::ostream& os,
                             const PrintPassConfig& config)
    : BasicBlockLevelPass(manager), os_(os), config_(config) {}

bool Passes::PrintPass::apply(IR::Function& function, IR::BasicBlock& block) {
  auto& liveness =
      manager_.get_analysis<LivenessAnalysis>().get_liveness_info();

  os_ << get_block_name(block) << ":\n";

  for (const auto& instruction : block.instructions) {
    print_live_info(liveness.get_data(instruction.get(), Position::BEFORE));

    os_ << "\t" << instruction->to_string() << "\n";

    print_live_info(liveness.get_data(instruction.get(), Position::AFTER));
  }

  if (block.is_end()) {
    return false;
  }

  // some branches should be printed
  os_ << "\tchildren:";
  for (auto child : block.children) {
    os_ << " " << (child == nullptr ? "null" : get_block_name(*child));
  }
  os_ << "\n";

  return false;
}

void Passes::PrintPass::before_function(const IR::Function& function) const {
  indices_.clear();

  os_ << fmt::format("{}({}):\n", function.name,
                     fmt::join(function.arguments, ", "));

  os_ << "entryblock: " << get_block_name(*function.begin_block) << std::endl;
}

void Passes::PrintPass::after_function(const IR::Function& function) const {
  os_ << "\n";
}

std::string Passes::PrintPass::get_block_name(
    const IR::BasicBlock& block) const {
  auto [itr, was_inserted] = indices_.emplace(&block, indices_.size());

  if (config_.print_blocks_addresses) {
    return fmt::format("bb.{} ({})", itr->second,
                       static_cast<const void*>(&block));
  } else {
    return fmt::format("bb.{}", itr->second);
  }
}

void Passes::PrintPass::print_live_info(
    const std::unordered_map<IR::Value, TemporaryLivenessState>& live) {
  if (!config_.print_live_info) {
    return;
  }

  for (auto [value, state] : live) {
    if (state == TemporaryLivenessState::LIVE) {
      os_ << value.to_string() << " ";
    }
  }
  os_ << "\n";
}
