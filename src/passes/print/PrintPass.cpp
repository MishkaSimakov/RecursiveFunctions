#include "PrintPass.h"

#include <algorithm>
#include <ranges>

#include "intermediate_representation/Function.h"
#include "passes/PassManager.h"
#include "passes/analysis/dominators/DominatorsAnalysis.h"

Passes::PrintPass::PrintPass(PassManager& manager, std::ostream& os)
    : ParentsFirstPass(manager), os_(os) {}

void Passes::PrintPass::process_block(IR::Function& function,
                                      IR::BasicBlock& block) {
  os_ << get_block_name(block) << ":\n";

  for (const auto& instruction : block.instructions) {
    os_ << "\t" << instruction->to_string() << "\n";
  }

  if (block.is_end()) {
    return;
  }

  // some branches should be printed
  os_ << "\tchildren:";
  for (auto child : block.children) {
    os_ << " " << (child == nullptr ? "null" : get_block_name(*child));
  }
  os_ << "\n";
}

void Passes::PrintPass::before_function(IR::Function& function) {
  indices_.clear();

  os_ << fmt::format("{}({}):\n", function.name,
                     fmt::join(function.arguments, ", "));
}

void Passes::PrintPass::after_function(IR::Function& function) { os_ << "\n"; }

std::string Passes::PrintPass::get_block_name(const IR::BasicBlock& block) {
  auto [itr, was_inserted] = indices_.emplace(&block, indices_.size());

  return "bb." + std::to_string(itr->second);
}
