#pragma once

#include "compilation/CompileTreeNodes.h"

namespace Compilation {
using std::vector, std::string, std::pair;

class BytecodeCompiler final : public Compiler {
  vector<list<Instruction>> compiled_functions_;
  list<Instruction> compiled_call_;
  size_t current_offset_{0};
  size_t argmin_parameter_position_{0};
  size_t recursion_parameter_position_{0};

  list<Instruction> result_;

  list<Instruction> compile_node(const std::unique_ptr<CompileNode>& node,
                                 size_t offset_change = 0) {
    current_offset_ += offset_change;
    node->accept(*this);
    current_offset_ -= offset_change;

    auto result = std::move(result_);
    result_.clear();

    return result;
  }

  static void offset_jumps(list<Instruction>& instructions, size_t offset) {
    for (auto& instruction : instructions) {
      if (instruction.type == InstructionType::POP_JUMP_IF_ZERO ||
          instruction.type == InstructionType::JUMP_IF_NONZERO) {
        instruction.argument += offset;
      }
    }
  }

  size_t get_recursion_parameter_position() const;
  size_t get_recursion_call_result_position() const;

  size_t get_argmin_parameter_position() const;

 public:
  static const vector<pair<string, size_t>> internal_functions;

  vector<Instruction> get_result() const;

  void compile(const ProgramNode&) override;
  void compile(const FunctionDefinitionNode&) override;
  void compile(const RecursiveFunctionDefinitionNode&) override;
  void compile(const RecursionParameterNode&) override;
  void compile(const ArgminCallNode&) override;
  void compile(const InternalFunctionDefinitionNode&) override;
  void compile(const ConstantNode&) override;
  void compile(const VariableNode&) override;
  void compile(const AsteriskNode&) override;
  void compile(const FunctionCallNode&) override;
  void compile(const SelfCallNode&) override;
};

inline const vector<pair<string, size_t>> BytecodeCompiler::internal_functions{
    {"successor", 1}, {"__add", 2}, {"__abs_diff", 2}};
}  // namespace Compilation
