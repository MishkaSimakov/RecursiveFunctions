#include "Printer.h"

#include "Function.h"

void IR::Printer::print_basic_block(const BasicBlock& basic_block) {
  for (const auto& instruction : basic_block.instructions) {
    os_ << prefix << instruction->to_string() << "\n";
  }

  if (basic_block.is_end()) {
    return;
  }

  // some branches should be printed
  std::cout << prefix << "children:";
  for (auto child : basic_block.children) {
    std::cout << " "
              << (child == nullptr ? "null"
                                   : std::to_string(get_block_index(*child)));
  }
  std::cout << "\n";
}

void IR::Printer::print_basic_blocks_recursively(
    const BasicBlock& basic_block) {
  size_t index = get_block_index(basic_block);

  auto& is_printed = used_[&basic_block].second;

  if (is_printed) {
    return;
  }

  is_printed = true;

  os_ << index << ":\n";
  print_basic_block(basic_block);

  for (auto child : basic_block.children) {
    if (child != nullptr) {
      print_basic_blocks_recursively(*child);
    }
  }
}

size_t IR::Printer::get_block_index(const BasicBlock& basic_block) {
  size_t index = used_.size();
  auto [itr, was_inserted] =
      used_.emplace(&basic_block, std::pair{index, false});

  if (!was_inserted) {
    index = itr->second.first;
  }

  return index;
}

void IR::Printer::print(const Program& program) {
  for (const auto& function : program.functions) {
    os_ << "function " << function.name << ":\n";

    if (function.begin_block != nullptr) {
      used_.clear();
      print_basic_blocks_recursively(*function.begin_block);
    }
  }
}
