#ifndef COMPILER_H
#define COMPILER_H
#include <list>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "Instructions.h"
#include "syntax/buffalo/SyntaxNode.h"

using std::vector, std::map, std::unordered_map, std::string, std::unique_ptr,
    std::list;

namespace Compilation {

struct Block {
  vector<Instruction> instructions;
  bool is_complete;
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
 * 1. For non-recursive function: in declaration, usage of variable that
 *    wasn't in arguments list.
 * 2. For recursive: same as 2, but function name may appear in function value.
 * 3. Usage of asterisk not inside argmin function.
 * 4. Usage of function before declaration.
 * 5. Usage of recursive function before full declaration (consists of zero-case
 *    declaration and general-case declaration).
 * 6. Wrong number of arguments in function call.
 * 7. Absence of asterisk inside argmin function.
 * 8. Call of argmin directly into another argmin (because of asterisk conflict)
 */
class Compiler {
  unordered_map<string, size_t> functions_indices_;
  vector<Block> functions_;

  static ValueT get_value_type(const SyntaxNode& node) {
    return std::stoi(node.value);
  }

  list<Instruction> compile_assignment(const SyntaxNode& node) {
    return {};
  }

  list<Instruction> compile_function_call(const SyntaxNode& node) {
    if (node.type == SyntaxNodeType::CONSTANT) {
      return {Instruction(InstructionType::LOAD_CONST, get_value_type(node))};
    }

    // node type is function
    list<Instruction> result;
    auto& children = node.children;

    for (auto itr = children.rbegin(); itr != children.rend(); ++itr) {
      auto instructions = compile_function_call(**itr);

      result.splice(result.end(), instructions);
    }

    const string& function_name = node.value;
    auto itr = functions_indices_.find(function_name);

    if (itr == functions_indices_.end()) {
      throw std::runtime_error("Usage of function before declaration.");
    }

    result.emplace_back(InstructionType::CALL_FUNCTION, itr->second);

    return result;
  }

 public:
  vector<Instruction> compile(const SyntaxNode& node) {
    for (auto& expression : node.children) {
      auto result = expression->type == SyntaxNodeType::ASSIGNMENT
                        ? compile_assignment(*expression)
                        : compile_function_call(*expression);
    }

    return {{InstructionType::LOAD_CONST, 1}, {InstructionType::HALT}};
  }
};
}  // namespace Compilation

#endif  // COMPILER_H
