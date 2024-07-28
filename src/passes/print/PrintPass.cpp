#include "PrintPass.h"

#include <algorithm>
#include <ranges>

#include "intermediate_representation/Function.h"

Passes::PrintPass::PrintPass(PassManager& manager, std::ostream& os)
    : ParentsFirstPass(manager), os_(os) {}

void Passes::PrintPass::process_block(IR::Function& function,
                                      IR::BasicBlock& block) {
  size_t index = get_block_index(block);
  os_ << "bb." << index << ":\n";

  for (const auto& instruction : block.instructions) {
    os_ << "\t" << instruction->to_string() << "\n";
  }

  if (block.is_end()) {
    return;
  }

  // some branches should be printed
  os_ << "\tchildren:";
  for (auto child : block.children) {
    os_ << " "
        << (child == nullptr ? "null"
                             : "bb." + std::to_string(get_block_index(*child)));
  }
  os_ << "\n";
}

void Passes::PrintPass::before_function(IR::Function& function) {
  indices_.clear();

  os_ << function.name << "(";

  auto arguments = std::views::iota(0) |
                   std::views::take(function.arguments_count) |
                   std::views::transform([](size_t index) {
                     return IR::Temporary{index}.to_string();
                   });

  os_ << fmt::format("{}", fmt::join(arguments, ", ")) << "):\n";
}

void Passes::PrintPass::after_function(IR::Function& function) { os_ << "\n"; }

size_t Passes::PrintPass::get_block_index(const IR::BasicBlock& block) {
  auto [itr, was_inserted] = indices_.emplace(&block, indices_.size());

  return itr->second;
}
