#ifndef COMPILER_H
#define COMPILER_H
#include <list>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "Instructions.h"
#include "functions/FastAbsoluteDifference.h"
#include "functions/FastAdd.h"
#include "syntax/buffalo/SyntaxNode.h"

using std::vector, std::map, std::unordered_map, std::string, std::unique_ptr,
    std::list;

namespace Compilation {
enum class FunctionState { COMPLETED, ONLY_ZERO_CASE, ONLY_GENERAL_CASE };

struct FunctionInfo {
  size_t id;
  size_t variables_count;
  FunctionState state;

  bool is_recursive;
  bool use_previous_value;
};

enum class VariableType { VARIABLE, RECURSION_PARAMETER, RECURSION_CALL };

struct VariableInfo {
  size_t id;
  VariableType type;
  bool was_used = false;
};

class BytecodeCompilerOld {
  unordered_map<string, FunctionInfo> functions_info_;
  vector<Block> functions_;
  list<Instruction> function_call_;

  static size_t get_value_type(const SyntaxNode& node) {
    return std::stoi(node.value);
  }

  static bool is_function_signature_recursive(const SyntaxNode& info_node) {
    return info_node.children.back()->type ==
           SyntaxNodeType::RECURSION_PARAMETER;
  }

  static bool is_recursive_zero_case(const SyntaxNode& info_node) {
    return info_node.children.back()->value == "0";
  }

  void compile_assignment(const SyntaxNode& node) {
    unordered_map<string, VariableInfo> variables;

    const auto& info_node = *node.children[0];
    const auto& value_node = *node.children[1];

    size_t variables_count = info_node.children.size();
    const string& function_name = info_node.value;

    auto function_info = functions_info_.find(function_name);
    bool function_was_found = function_info != functions_info_.end();
    bool is_recursive = is_function_signature_recursive(info_node);

    if (!is_recursive && function_was_found) {
      throw new std::runtime_error("Redefinition of function.");
    }

    for (size_t i = 0; i < variables_count; ++i) {
      string varname = info_node.children[i]->value;

      if (is_recursive && varname == info_node.value) {
        throw std::runtime_error(
            "Recursive function can not contain it's name as variable.");
      }

      if (variables.contains(varname)) {
        throw std::runtime_error(
            "All function variables names must be unique.");
      }

      // we store variables in reversed order
      variables.emplace(
          std::move(varname),
          VariableInfo{variables_count - i - 1, VariableType::VARIABLE});
    }

    if (is_recursive) {
      auto& last_varname = info_node.children.back()->value;
      variables[last_varname].type = VariableType::RECURSION_PARAMETER;

      variables.emplace(function_name,
                        VariableInfo{0, VariableType::RECURSION_CALL});
    }

    auto instructions =
        compile_function(value_node, false, false, variables, 0);
    instructions.emplace_back(InstructionType::RETURN);

    if (is_recursive && variables[function_name].was_used) {
      functions_info_[function_name].use_previous_value = true;
    }

    if (is_recursive && function_was_found) {
      if (function_info->second.variables_count != variables_count) {
        throw std::runtime_error(
            "Declaration of recursive function case with wrong number of "
            "arguments");
      }

      bool is_zero_case = is_recursive_zero_case(info_node);
      complete_recursive_function(function_info->second,
                                  std::move(instructions), is_zero_case);
      return;
    }

    auto state = FunctionState::COMPLETED;
    if (is_recursive) {
      state = is_recursive_zero_case(info_node)
                  ? FunctionState::ONLY_ZERO_CASE
                  : FunctionState::ONLY_GENERAL_CASE;
    }

    functions_info_[function_name].id = functions_.size();
    functions_info_[function_name].variables_count = variables_count;
    functions_info_[function_name].state = state;
    functions_info_[function_name].is_recursive = is_recursive;

    functions_.emplace_back(std::move(instructions));
  }

  void complete_recursive_function(FunctionInfo& info,
                                   list<Instruction> instructions,
                                   bool is_zero_case) {
    auto& block = functions_[info.id];
    list<Instruction> zero_case_instructions = std::move(instructions);
    list<Instruction> general_case_instructions = std::move(block.instructions);

    if (is_zero_case) {
      if (info.state != FunctionState::ONLY_GENERAL_CASE) {
        throw std::runtime_error(
            "Recursive function definition must contain only one zero case and "
            "only one general case.");
      }
    } else {
      if (info.state != FunctionState::ONLY_ZERO_CASE) {
        throw std::runtime_error(
            "Recursive function definition must contain only one zero case and "
            "only one general case.");
      }

      std::swap(zero_case_instructions, general_case_instructions);
    }

    block.instructions.clear();

    if (info.use_previous_value) {
      size_t offset =
          zero_case_instructions.size() + general_case_instructions.size() - 2;

      block.instructions.splice(block.instructions.end(),
                                zero_case_instructions);
      block.instructions.pop_back();

      /* 1 */ block.instructions.emplace_back(InstructionType::LOAD, 0);

      // jump to the end
      /* 2 */ block.instructions.emplace_back(InstructionType::POP_JUMP_IF_ZERO,
                                              offset + 16);

      // load variables for recursion cycle
      /* 3 */ block.instructions.emplace_back(InstructionType::LOAD, 0);
      /* 4 */ block.instructions.emplace_back(InstructionType::DECREMENT);
      /* 5 */ block.instructions.emplace_back(InstructionType::LOAD_CONST, 0);
      /* 6 */ block.instructions.emplace_back(InstructionType::COPY, 2);

      block.instructions.splice(block.instructions.end(),
                                general_case_instructions);
      // remove RETURN (here we assume that all functions has only one return in
      // the end)
      block.instructions.pop_back();

      // leave cycle if we calculated last value
      /* 7 */ block.instructions.emplace_back(InstructionType::POP, 1);
      /* 8 */ block.instructions.emplace_back(InstructionType::COPY, 2);
      /* 9 */ block.instructions.emplace_back(InstructionType::POP_JUMP_IF_ZERO,
                                              offset + 13);

      // increment loop counter
      /* 10 */ block.instructions.emplace_back(InstructionType::INCREMENT, 1);
      /* 11 */ block.instructions.emplace_back(InstructionType::DECREMENT, 2);

      // jump back to cycle start
      /* 12 */ block.instructions.emplace_back(InstructionType::LOAD_CONST, 0);
      /* 13 */ block.instructions.emplace_back(
          InstructionType::POP_JUMP_IF_ZERO, zero_case_instructions.size() + 7);

      /* 14 */ block.instructions.emplace_back(InstructionType::POP, 1);
      /* 15 */ block.instructions.emplace_back(InstructionType::POP, 1);
      /* 16 */ block.instructions.emplace_back(InstructionType::POP, 1);

      /* 17 */ block.instructions.emplace_back(InstructionType::RETURN);
    } else {
      block.instructions.emplace_back(InstructionType::LOAD, 0);
      block.instructions.emplace_back(InstructionType::JUMP_IF_NONZERO,
                                      zero_case_instructions.size() + 3);

      block.instructions.splice(block.instructions.end(),
                                zero_case_instructions);
      block.instructions.pop_back();

      block.instructions.emplace_back(InstructionType::LOAD_CONST, 0);
      block.instructions.emplace_back(
          InstructionType::POP_JUMP_IF_ZERO,
          general_case_instructions.size() + zero_case_instructions.size() + 7);

      block.instructions.emplace_back(InstructionType::DECREMENT, 0);
      block.instructions.emplace_back(InstructionType::LOAD_CONST, 0);
      block.instructions.splice(block.instructions.end(),
                                general_case_instructions);
      block.instructions.pop_back();

      block.instructions.emplace_back(InstructionType::POP, 1);
      block.instructions.emplace_back(InstructionType::POP, 1);
      block.instructions.emplace_back(InstructionType::RETURN);
    }

    info.state = FunctionState::COMPLETED;
  }

  list<Instruction> compile_function(
      const SyntaxNode& node, bool only_constant_params, bool is_inside_argmin,
      unordered_map<string, VariableInfo>& variables_map, size_t offset) {
    if (node.type == SyntaxNodeType::CONSTANT) {
      return {{InstructionType::LOAD_CONST, get_value_type(node)}};
    }
    if (node.type == SyntaxNodeType::VARIABLE) {
      if (only_constant_params) {
        throw std::runtime_error(
            "Functions can only contain constants as arguments in this "
            "context.");
      }

      auto itr = variables_map.find(node.value);
      if (itr == variables_map.end()) {
        throw std::runtime_error(
            "Unknown variable inside function definition.");
      }

      itr->second.was_used = true;

      auto& variable_info = itr->second;
      if (variable_info.type == VariableType::RECURSION_PARAMETER) {
        return {{InstructionType::COPY, offset + 1}};
      }

      if (variable_info.type == VariableType::RECURSION_CALL) {
        return {{InstructionType::COPY, offset}};
      }

      return {{InstructionType::LOAD, variable_info.id}};
    }
    if (node.type == SyntaxNodeType::ASTERISK) {
      if (!is_inside_argmin) {
        throw std::runtime_error(
            "Asterisk may only be used inside argmin function call.");
      }

      return {{InstructionType::COPY, offset}};
    }

    // if node type is function
    list<Instruction> result;

    const string& function_name = node.value;
    auto& children = node.children;

    if (function_name == "argmin") {
      return compile_argmin(node, only_constant_params, is_inside_argmin,
                            variables_map, offset);
    }

    if (function_name != "successor") {
      auto function_info = functions_info_.find(function_name);
      if (function_info == functions_info_.end()) {
        throw std::runtime_error("Usage of function before definition.");
      }

      auto variables_count = function_info->second.variables_count;

      if (variables_count != children.size()) {
        throw std::runtime_error(
            "Call of function with wrong number of arguments");
      }

      ++offset;

      result.emplace_back(InstructionType::LOAD_CALL, function_info->second.id);
    }

    for (size_t i = 0; i < children.size(); ++i) {
      size_t child_id = children.size() - i - 1;
      auto instructions =
          compile_function(*children[child_id], only_constant_params,
                           is_inside_argmin, variables_map, offset);

      ++offset;

      result.splice(result.end(), instructions);
    }

    if (function_name == "successor") {
      result.emplace_back(InstructionType::INCREMENT);
    } else {
      result.emplace_back(InstructionType::CALL_FUNCTION);
    }

    return result;
  }

  void compile_function_call(const SyntaxNode& node) {
    if (!function_call_.empty()) {
      throw std::runtime_error(
          "There must be only one function call in program.");
    }

    unordered_map<string, VariableInfo> temp;
    function_call_ = compile_function(node, true, false, temp, 0);
    function_call_.emplace_back(InstructionType::HALT);
  }

  void load_system_functions() {
    // fast add
    functions_info_["__add"] = {functions_.size(), 2, FunctionState::COMPLETED};
    functions_.emplace_back(fast_add_instructions);

    functions_info_["__abs_diff"] = {functions_.size(), 2,
                                     FunctionState::COMPLETED};
    functions_.emplace_back(fast_absolute_difference_instructions);
  }

  vector<Instruction> get_combined_instructions() {
    vector<Instruction> result;
    vector<size_t> functions_offsets;
    functions_offsets.resize(functions_.size());

    result.insert(result.end(), function_call_.begin(), function_call_.end());

    for (size_t func_index = 0; func_index < functions_.size(); ++func_index) {
      size_t offset = result.size();
      functions_offsets[func_index] = offset;

      for (auto instruction : functions_[func_index].instructions) {
        if (instruction.type == InstructionType::POP_JUMP_IF_ZERO ||
            instruction.type == InstructionType::JUMP_IF_NONZERO) {
          instruction.argument += offset;
        }

        result.push_back(instruction);
      }
    }

    offset_calls(result, functions_offsets);

    return result;
  }

  list<Instruction> compile_argmin(
      const SyntaxNode& node, bool only_constant_params, bool is_inside_argmin,
      unordered_map<string, VariableInfo>& variables_map, size_t offset) {
    if (is_inside_argmin) {
      throw std::runtime_error(
          "Argmin function cannot be used inside another argmin call");
    }

    auto& wrapped = *node.children.front();

    auto child_instructions = compile_function(wrapped, only_constant_params,
                                               true, variables_map, offset);

    list<Instruction> result;

    result.emplace_back(InstructionType::LOAD_CONST, 0);

    result.splice(result.end(), child_instructions);

    result.emplace_back(InstructionType::POP_JUMP_IF_ZERO, result.size() + 4);
    result.emplace_back(InstructionType::INCREMENT);
    result.emplace_back(InstructionType::LOAD_CONST, 0);
    result.emplace_back(InstructionType::POP_JUMP_IF_ZERO, 1);

    return result;
  }

  static void offset_calls(vector<Instruction>& instructions,
                           const vector<size_t>& offsets) {
    for (auto& instruction : instructions) {
      if (instruction.type == InstructionType::LOAD_CALL) {
        instruction.argument = offsets[instruction.argument];
      }
    }
  }

 public:
  vector<Instruction> compile(const SyntaxNode& node) {
    load_system_functions();

    for (auto& expression : node.children) {
      if (expression->type == SyntaxNodeType::ASSIGNMENT) {
        compile_assignment(*expression);
      } else {
        compile_function_call(*expression);
      }
    }

    if (function_call_.empty()) {
      throw std::runtime_error("Program must contain function call.");
    }

    auto result = get_combined_instructions();
    return result;
  }
};
}  // namespace Compilation

#endif  // COMPILER_H
