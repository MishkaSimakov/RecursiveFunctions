#include "Printer.h"

void IR::Printer::print_basic_block(const BasicBlock& basic_block) {
  for (const auto& instruction : basic_block.instructions) {
    os_ << prefix << instruction->to_string() << "\n";
  }

  if (basic_block.is_end()) {
    os_ << prefix << "return " << basic_block.end_value.to_string() << "\n";
    return;
  }

  // some branches should be printed
  size_t left_index = get_block_index(*basic_block.children.first);

  if (basic_block.children.second == nullptr) {
    os_ << prefix << "jump " << left_index << "\n";
  } else {
    size_t right_index = get_block_index(*basic_block.children.second);

    os_ << prefix
        << fmt::format("jump {} == 0 ? {} : {}\n",
                       basic_block.end_value.to_string(), left_index,
                       right_index);
  }
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

  if (basic_block.children.first != nullptr) {
    print_basic_blocks_recursively(*basic_block.children.first);
  }

  if (basic_block.children.second != nullptr) {
    print_basic_blocks_recursively(*basic_block.children.second);
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
