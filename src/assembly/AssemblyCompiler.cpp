#include "AssemblyCompiler.h"

#include "intermediate_representation/Function.h"

namespace Assembly {
std::unordered_map<std::type_index, AssemblyCompiler::AssemblyGeneratorT>
    AssemblyCompiler::generators_ = {
        {typeid(IR::Addition),
         [](CompileDTO data) { data.result.emplace_back("add"); }}};

auto AssemblyCompiler::decorate_function(const std::string& name,
                                         std::list<AssemblyInstruction> body,
                                         bool is_leaf) {
  std::list<AssemblyInstruction> result;

  // function label
  std::list label = {
      AssemblyInstruction(".global " + name),
      AssemblyInstruction(".align 4"),
      AssemblyInstruction(name + ":"),
  };

  result.splice(result.begin(), label);

  if (!is_leaf) {
    result.emplace_back("stp", "x29", "x30", "[sp, #-16]!");
    result.emplace_back("mov", "x29", "sp");
  }

  result.splice(result.end(), body);

  if (!is_leaf) {
    result.emplace_back("ldp", "x29", "x30", "[sp]", "#16");
  }

  result.emplace_back("ret");

  return result;
}

void AssemblyCompiler::compile_basic_block(const IR::Function& function,
                                           const IR::BasicBlock* block) {
  for (auto& instruction : block->instructions) {
    // CompileDTO dto{function, block, };
    // generators_[typeid(instruction)](*instruction, block);
  }
}

void AssemblyCompiler::compile_function(const IR::Function& function) {
  storage_.clear();

  function.traverse_blocks([this, &function](const IR::BasicBlock* block) {
    compile_basic_block(function, block);
  });

  auto ordering = arrange_blocks(function);

  std::list<AssemblyInstruction> result;
  for (auto itr = ordering.begin(); itr != ordering.end(); ++itr) {
    auto* block = *itr;
    auto& block_instructions = storage_[block];

    result.splice(result.end(), block_instructions);

    // by default we have 2 jumps in the end of a basic block
    // if one of the children are right under this block we should remove
    // one of the jumps and just fallthrough
    auto next = std::next(itr);

    if (block->is_end() || next == ordering.end()) {
      continue;
    }

    if (block->children[1] == nullptr) {
      // this is unconditional branch to another block
      if (*next == block->children[1]) {
        result.pop_back();
      }

      continue;
    }

    // this block has conditional jump
    // first instruction is jump to the first child
    // second - to the second
    if (*next == block->children[0]) {
      result.erase(std::prev(result.end(), 2));
    } else if (*next == block->children[1]) {
      result.pop_back();
    }
  }
}

std::vector<const IR::BasicBlock*> AssemblyCompiler::arrange_blocks(
    const IR::Function&) {}
}  // namespace Assembly
