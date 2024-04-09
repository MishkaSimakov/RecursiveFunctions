#ifndef COMPILER_H
#define COMPILER_H
#include <list>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "Instructions.h"
#include "functions/Successor.h"
#include "syntax/buffalo/SyntaxNode.h"

using std::vector, std::map, std::unordered_map, std::string, std::unique_ptr,
    std::list;

namespace Compilation {

enum class BlockState { COMPLETED, ONLY_ZERO_CASE, ONLY_GENERAL_CASE };

struct Block {
  list<Instruction> instructions;
  BlockState state;

  Block(list<Instruction> instructions, BlockState state)
      : instructions(std::move(instructions)), state(state) {}
};

/*
 * This class should transform all variables and function names into numbers
 * Rules:
 * - variables in function declaration are numerated in the same order as in
 *   function arguments list: f(x, y, z) -> f(0, 1, 2).
 * - functions are numerated in top to bottom order, recursive functions
 *   have same number associated with them.
 * All errors concerning functions and variables names must occure there.
 * List of errors:
 * 1. For non-recursive function: in definition, usage of variable that
 *    wasn't in arguments list.
 * 2. For recursive: same as 2, but function name may appear in function value.
 * 3. Double usage of variable in function.
 * 4. Usage of asterisk not inside argmin function.
 * 5. Usage of function before definition.
 * 6. Usage of recursive function before full definition (consists of zero-case
 *    definition and general-case definition).
 * 7. Wrong number of arguments in function call.
 * 8. Absence of asterisk inside argmin function.
 * 9. Call of argmin directly into another argmin (because of asterisk conflict)
 */
class Compiler {
  unordered_map<string, size_t> functions_indices_;
  vector<Block> functions_;
  list<Instruction> function_call_;

  static ValueT get_value_type(const SyntaxNode& node) {
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
    unordered_map<string, size_t> variables;

    const auto& info_node = *node.children[0];
    const auto& value_node = *node.children[1];

    size_t variables_count = info_node.children.size();
    const string& function_name = info_node.value;

    auto function_index_itr = functions_indices_.find(function_name);
    bool is_recursive = is_function_signature_recursive(info_node);

    if (!is_recursive && function_index_itr != functions_indices_.end()) {
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
      variables.emplace(std::move(varname), variables_count - i - 1);
    }

    auto instructions = compile_function(value_node, false, false, variables);
    instructions.emplace_back(InstructionType::RETURN);

    if (is_recursive && function_index_itr != functions_indices_.end()) {
      bool is_zero_case = is_recursive_zero_case(info_node);
      complete_recursive_function(function_index_itr->second,
                                  std::move(instructions), is_zero_case);
      return;
    }

    auto state = BlockState::COMPLETED;
    if (is_recursive) {
      state = is_recursive_zero_case(info_node) ? BlockState::ONLY_ZERO_CASE
                                                : BlockState::ONLY_GENERAL_CASE;
    }

    Block block(std::move(instructions), state);

    functions_indices_[function_name] = functions_.size();
    functions_.push_back(std::move(block));
  }

  void complete_recursive_function(size_t index, list<Instruction> instructions,
                                   bool is_zero_case) {
    list<Instruction> zero_case_instructions;
    list<Instruction> general_case_instructions;
    auto& block = functions_[index];

    if (is_zero_case) {
      if (block.state != BlockState::ONLY_GENERAL_CASE) {
        throw std::runtime_error(
            "Recursive function definition must contain only one zero case and "
            "only one general case.");
      }

      zero_case_instructions = std::move(instructions);
      general_case_instructions = std::move(block.instructions);
    } else {
      if (block.state != BlockState::ONLY_ZERO_CASE) {
        throw std::runtime_error(
            "Recursive function definition must contain only one zero case and "
            "only one general case.");
      }

      zero_case_instructions = std::move(block.instructions);
      general_case_instructions = std::move(instructions);
    }

    block.instructions = {
        {InstructionType::LOAD, 0},
        {InstructionType::JUMP_IF_ZERO, 2 + general_case_instructions.size()}};

    block.instructions.splice(block.instructions.end(),
                              general_case_instructions);
    block.instructions.splice(block.instructions.end(), zero_case_instructions);

    block.state = BlockState::COMPLETED;
  }

  list<Instruction> compile_function(
      const SyntaxNode& node, bool only_constant_params, bool is_inside_argmin,
      const unordered_map<string, size_t>& variables_map) {
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

      return {{InstructionType::LOAD, itr->second}};
    }
    if (node.type == SyntaxNodeType::ASTERISK) {
      throw std::runtime_error("Not implemented yet!");
    }

    // if node type is function
    list<Instruction> result;
    auto& children = node.children;

    for (auto itr = children.rbegin(); itr != children.rend(); ++itr) {
      auto instructions = compile_function(**itr, only_constant_params,
                                           is_inside_argmin, variables_map);

      result.splice(result.end(), instructions);
    }

    const string& function_name = node.value;

    auto itr = functions_indices_.find(function_name);

    if (itr == functions_indices_.end()) {
      throw std::runtime_error("Usage of function before definition.");
    }

    result.emplace_back(InstructionType::CALL_FUNCTION, itr->second,
                        children.size());

    return result;
  }

  void compile_function_call(const SyntaxNode& node) {
    if (!function_call_.empty()) {
      throw std::runtime_error(
          "There must be only one function call in program.");
    }

    function_call_ = compile_function(node, true, false, {});
    function_call_.emplace_back(InstructionType::HALT);
  }

  void load_system_functions() {
    // successor
    functions_indices_["successor"] = functions_.size();
    functions_.emplace_back(successor_instructions, BlockState::COMPLETED);
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
        if (instruction.type == InstructionType::JUMP_IF_ZERO ||
            instruction.type == InstructionType::JUMP_IF_NONZERO) {
          instruction.first_argument += offset;
        }

        result.push_back(instruction);
      }
    }

    offset_calls(result, functions_offsets);

    return result;
  }

  static void offset_calls(vector<Instruction>& instructions,
                           const vector<size_t>& offsets) {
    for (auto& instruction : instructions) {
      if (instruction.type == InstructionType::CALL_FUNCTION) {
        instruction.first_argument = offsets[instruction.first_argument];
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

    auto result = get_combined_instructions();
    return result;
  }
};
}  // namespace Compilation

#endif  // COMPILER_H
