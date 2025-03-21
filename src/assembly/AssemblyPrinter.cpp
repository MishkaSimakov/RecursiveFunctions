#include "AssemblyPrinter.h"

#include "GreedyBlocksOrdering.h"
#include "InstructionPrinter.h"

void Assembly::AssemblyPrinter::before_function(
    const InstructionContext& context) {
  std::string name = mangle_function_name(context.function);

  // function label
  result.push_back(".p2align 4");
  result.push_back(".global " + name);
  result.push_back(name + ":");

  // save frame and link registers
  if (!context.function.is_leaf()) {
    result.push_back("stp x29, x30, [sp, #-16]!");
    result.push_back("mov x29, sp");
  }

  print_callee_saved_registers(CalleeSavedOperationType::STORE, context);

  auto begin_block = context.function.begin_block;
  if (context.ordering.front() != begin_block) {
    result.push_back("b " + context.labels.at(begin_block));
  }
}

void Assembly::AssemblyPrinter::after_function(
    const InstructionContext& context) {
  print_callee_saved_registers(CalleeSavedOperationType::LOAD, context);

  if (!context.function.is_leaf()) {
    result.push_back("ldp x29, x30, [sp], #16");
  }

  result.push_back("ret");
}

void Assembly::AssemblyPrinter::print_callee_saved_registers(
    CalleeSavedOperationType operation, const InstructionContext& context) {
  // store callee-saved registers
  if (context.callee_saved_registers.empty()) {
    return;
  }

  auto reg_pair_action =
      operation == CalleeSavedOperationType::STORE ? "stp" : "ldp";
  auto reg_action =
      operation == CalleeSavedOperationType::STORE ? "str" : "ldr";

  size_t sp_offset = context.callee_saved_registers.size();
  sp_offset += sp_offset % 2;
  sp_offset *= 8;

  if (operation == CalleeSavedOperationType::STORE) {
    result.push_back(fmt::format(
        "sub sp, sp, {}",
        print_value(IR::Value(sp_offset, IR::ValueType::CONSTANT))));
  }

  size_t callee_saved_count = context.callee_saved_registers.size();
  for (size_t i = 0; i < callee_saved_count; i += 2) {
    auto stack_index = IR::Value(i, IR::ValueType::STACK_INDEX);

    if (i + 1 < callee_saved_count) {
      // process two at once
      result.push_back(
          fmt::format("{} {}, {}, {}", reg_pair_action,
                      print_value(context.callee_saved_registers[i]),
                      print_value(context.callee_saved_registers[i + 1]),
                      print_value(stack_index)));
    } else {
      result.push_back(
          fmt::format("{} {}, {}", reg_action,
                      print_value(context.callee_saved_registers[i]),
                      print_value(stack_index)));
    }
  }

  if (operation == CalleeSavedOperationType::LOAD) {
    result.push_back(fmt::format(
        "add sp, sp, {}",
        print_value(IR::Value(sp_offset, IR::ValueType::CONSTANT))));
  }
}

std::string Assembly::AssemblyPrinter::mangle_function_name(
    const IR::Function& function) {
  return mangle_function_name(function.name);
}

std::string Assembly::AssemblyPrinter::mangle_function_name(
    const std::string& name) {
  return "_" + name;
}

std::string Assembly::AssemblyPrinter::print_value(IR::Value value) {
  switch (value.type) {
    case IR::ValueType::CONSTANT:
      return fmt::format("#{}", value.value);
    case IR::ValueType::VIRTUAL_REGISTER:
      throw std::runtime_error("Too late for virtual registers");
    case IR::ValueType::BASIC_REGISTER:
      return fmt::format("x{}", value.value);
    case IR::ValueType::CALLEE_SAVED_REGISTER:
      return fmt::format("x{}", value.value + 19);
    case IR::ValueType::STACK_INDEX:
      return fmt::format("[sp, #{}]", value.value * 8);
  }

  throw std::runtime_error("Unknown value type");
}

std::vector<std::string> Assembly::AssemblyPrinter::print() {
  result.clear();

  for (auto& function : program_.functions) {
    InstructionContext context(function);

    for (IR::Value value : context.function.temporaries) {
      if (value.type == IR::ValueType::CALLEE_SAVED_REGISTER) {
        context.callee_saved_registers.push_back(value);
      }
    }

    context.ordering = GreedyBlocksOrdering().get_order(function);

    for (size_t i = 0; i < context.ordering.size(); ++i) {
      context.labels[context.ordering[i]] =
          fmt::format("{}.{}", context.function.name, i);
    }

    before_function(context);

    for (size_t i = 0; i < context.ordering.size(); ++i) {
      context.block_index = i;

      const IR::BasicBlock* block = context.ordering[i];

      result.push_back(context.labels[block] + ":");

      for (auto& instruction : block->instructions) {
        auto instr = InstructionPrinter().apply(*instruction, context);

        if (!instr.empty()) {
          std::copy(std::make_move_iterator(instr.begin()),
                    std::make_move_iterator(instr.end()),
                    std::back_inserter(result));
        }
      }

      if (block->has_one_child() && !context.is_next(block->children[0])) {
        result.push_back(
            fmt::format("b {}", context.labels.at(block->children[0])));
      }

      if (block->is_end()) {
        after_function(context);
      }
    }
  }

  return std::move(result);
}
