#pragma once

#include "AssemblyInstruction.h"
#include "compilation/CompileTreeNodes.h"

namespace Compilation {
using std::vector, std::string, std::pair;

class BytecodeCompiler final : public CompileTreeVisitor {
  vector<list<AssemblyInstruction>> compiled_functions_;
  list<AssemblyInstruction> compiled_call_;

  // when function call is compiled this variable stores index of current
  // argument
  size_t current_argument_offset{0};

  list<AssemblyInstruction> result_;

  static bool is_function_call(const CompileNode& node) {
    return dynamic_cast<const FunctionCallNode*>(&node) != nullptr;
  }

  list<AssemblyInstruction> decorate_function(string name,
                                              list<AssemblyInstruction> body,
                                              bool is_leaf = false) const {
    list<AssemblyInstruction> result;

    // function label
    list label = {
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

  static string get_register(size_t id) {
    string result = "x";
    result += std::to_string(id);
    return result;
  }

  static string get_short_register(size_t id) {
    string result = "w";
    result += std::to_string(id);
    return result;
  }

  static string get_constant(int value) {
    string result = "#";
    result += std::to_string(value);
    return result;
  }

  static string get_mangled_name(const string& name) { return "_" + name; }

  list<AssemblyInstruction> compile_node(
      const std::unique_ptr<CompileNode>& node, size_t offset_change = 0) {
    node->accept(*this);

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

  vector<AssemblyInstruction> get_result() const;

  void visit(const ProgramNode&) override;
  void visit(const FunctionDefinitionNode&) override;
  void visit(const RecursiveFunctionDefinitionNode&) override;
  void visit(const RecursionParameterNode&) override;
  void visit(const ArgminCallNode&) override;
  void visit(const InternalFunctionDefinitionNode&) override;
  void visit(const ConstantNode&) override;
  void visit(const VariableNode&) override;
  void visit(const AsteriskNode&) override;
  void visit(const FunctionCallNode&) override;
  void visit(const SelfCallNode&) override;
};

inline const vector<pair<string, size_t>> BytecodeCompiler::internal_functions{
    {"successor", 1}, {"__add", 2}, {"__abs_diff", 2}};
}  // namespace Compilation
