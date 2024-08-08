#include "AssemblyPrinter.h"

#include "InstructionPrinter.h"

void Assembly::AssemblyPrinter::before_function(const IR::Function& function) {
  std::string name = mangle_function_name(function);

  // function label
  result.push_back(".p2align 4");
  result.push_back(".global " + name);
  result.push_back(name + ":");

  // save frame and link registers
  result.push_back("stp x29, x30, [sp, #-16]!");
  result.push_back("mov x29, sp");

  // store callee-saved registers
}

void Assembly::AssemblyPrinter::after_function(const IR::Function& function) {
  if (function.name == function.entrypoint) {
    // print result
    result.push_back("sub sp, sp, #16");
    result.push_back("str x0, [sp]");
    result.push_back("adrp x0, format@PAGE");
    result.push_back("add x0, x0, format@PAGEOFF");
    result.push_back("mov x1, sp");
    result.push_back("bl _printf");
    result.push_back("add sp, sp, #16");
    result.push_back("mov x0, #0");
  }

  result.push_back("ldp x29, x30, [sp], #16");
  result.push_back("ret");
}

std::string Assembly::AssemblyPrinter::mangle_function_name(
    const IR::Function& function) {
  return mangle_function_name(function.name);
}

std::string Assembly::AssemblyPrinter::mangle_function_name(
    const std::string& name) {
  return "_" + name;
}

std::vector<std::string> Assembly::AssemblyPrinter::print() {
  result.clear();

  for (auto& function : program_.functions) {
    before_function(function);

    std::unordered_map<const IR::BasicBlock*, std::string> labels;
    std::vector<const IR::BasicBlock*> ordering;

    function.traverse_blocks(
        [&ordering, &labels, &function](const IR::BasicBlock* block) {
          ordering.push_back(block);
          labels[block] = fmt::format("{}.{}", function.name, ordering.size());
        });

    InstructionContext context{labels, ordering, 0};

    for (size_t i = 0; i < ordering.size(); ++i) {
      context.block_index = i;

      const IR::BasicBlock* block = ordering[i];

      result.push_back(labels[block] + ":");

      for (auto& instruction : block->instructions) {
        auto str = InstructionPrinter().apply(*instruction, context);

        if (!str.empty()) {
          result.push_back(std::move(str));
        }
      }

      if (block->is_end()) {
        after_function(function);
      }
    }
  }

  result.push_back(".data");
  result.push_back("format: .asciz \"Result: %i\n\"");

  return std::move(result);
}
