#ifndef FUNCTIONSEXTRACTOR_H
#define FUNCTIONSEXTRACTOR_H

#include <unordered_map>
#include <vector>

#include "Compiler.h"
#include "compilation/Instructions.h"
#include "syntax/RecursiveFunctionsSyntax.h"
#include "syntax/buffalo/SyntaxNode.h"

using std::vector, std::unordered_map;

namespace Compilation {
class Compiler;
struct FunctionCompileNode;
struct ArgminCompileNode;
struct RecursiveFunctionCompileNode;
struct VariableCompileNode;
struct ConstantCompileNode;

struct CompileNode {
  virtual list<Instruction> compile(Compiler&) = 0;
  virtual ~CompileNode() = default;
};

class Compiler {
 public:
  virtual list<Instruction> compile(const FunctionCompileNode&) = 0;
  virtual list<Instruction> compile(const ArgminCompileNode&) = 0;
  virtual list<Instruction> compile(const RecursiveFunctionCompileNode&) = 0;
  virtual list<Instruction> compile(const VariableCompileNode&) = 0;
  virtual list<Instruction> compile(const ConstantCompileNode&) = 0;
  virtual ~Compiler() = default;
};

struct FunctionCallCompileNode final : CompileNode {
  size_t index;
  vector<unique_ptr<SyntaxNode>> children;

  explicit FunctionCallCompileNode(size_t index) : index(index) {}

  list<Instruction> compile(Compiler& compiler) override {
    return compiler.compile(*this);
  }
};

struct RecursiveFunctionCompileNode final : CompileNode {
  unique_ptr<CompileNode> zero_case_node;
  unique_ptr<CompileNode> general_case_node;

  list<Instruction> compile(Compiler& compiler) override {
    return compiler.compile(*this);
  }
};

struct ArgminCompileNode final : CompileNode {
  list<Instruction> compile(Compiler& compiler) override {
    return compiler.compile(*this);
  }
};

struct VariableCompileNode final : CompileNode {
  size_t variable_index;

  explicit VariableCompileNode(size_t index) : variable_index(index) {}

  list<Instruction> compile(Compiler& compiler) override {
    return compiler.compile(*this);
  }
};

struct ConstantCompileNode final : CompileNode {
  ValueT constant_value;

  explicit ConstantCompileNode(ValueT value) : constant_value(value) {}

  list<Instruction> compile(Compiler& compiler) override {
    return compiler.compile(*this);
  }
};

struct CompileTree {
  vector<unique_ptr<CompileNode>> assignments;
  unique_ptr<CompileNode> call;
};

class CompileTreeBuilder {
  CompileTree compile_tree_;
  unordered_map<string, size_t> functions_indices_;
  vector<Block> functions_;

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

  void add_compile_node_for_assignment(const SyntaxNode& node) {
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

    auto function_value_compile_node =
        get_function_call_compile_node(value_node, false, false, variables);

    if (is_recursive && function_index_itr != functions_indices_.end()) {
      bool is_zero_case = is_recursive_zero_case(info_node);
      complete_recursive_function_node(function_index_itr->second,
                                       std::move(function_value_compile_node),
                                       is_zero_case);
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

  void complete_recursive_function_node(size_t index,
                                        unique_ptr<CompileNode> compile_node,
                                        bool is_zero_case) {
    auto& recursive_node = static_cast<RecursiveFunctionCompileNode&>(
        *compile_tree_.assignments[index]);

    if (is_zero_case) {
      if (recursive_node.zero_case_node != nullptr) {
        throw std::runtime_error(
            "Recursive function definition must contain only one zero case and "
            "only one general case.");
      }

      recursive_node.zero_case_node = std::move(compile_node);
    } else {
      if (recursive_node.general_case_node != nullptr) {
        throw std::runtime_error(
            "Recursive function definition must contain only one zero case and "
            "only one general case.");
      }

      recursive_node.general_case_node = std::move(compile_node);
    }
  }

  unique_ptr<CompileNode> get_function_call_compile_node(
      const SyntaxNode& node, bool only_constant_params, bool is_inside_argmin,
      const unordered_map<string, size_t>& variables_map) {
    if (node.type == SyntaxNodeType::CONSTANT) {
      return std::make_unique<ConstantCompileNode>(get_value_type(node));
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

      return std::make_unique<VariableCompileNode>(itr->second);
    }
    if (node.type == SyntaxNodeType::ASTERISK) {
      if (!is_inside_argmin) {
        throw std::runtime_error(
            "Asterisk may only appear inside argmin function call.");
      }

      throw std::runtime_error("Not implemented yet!");
    }

    // if node type is function
    const string& function_name = node.value;
    auto function_index_itr = functions_indices_.find(function_name);

    if (function_index_itr == functions_indices_.end()) {
      throw std::runtime_error("Usage of function before definition.");
    }

    auto result =
        std::make_unique<FunctionCallCompileNode>(function_index_itr->second);
    auto& children = node.children;

    for (auto itr = children.rbegin(); itr != children.rend(); ++itr) {
      auto instructions = get_function_call_compile_node(
          **itr, only_constant_params, is_inside_argmin, variables_map);

      result->children
    }

    result.emplace_back(InstructionType::CALL_FUNCTION, itr->second,
                        children.size());

    return result;
  }

  void add_compile_node_for_function_call(const SyntaxNode& node) {
    if (compile_tree_.call != nullptr) {
      throw std::runtime_error(
          "There must be only one function call in program.");
    }

    compile_tree_.call = compile_function(node, true, false, {});
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
  void build(const SyntaxNode& node) {
    for (auto& expression : node.children) {
      if (expression->type == SyntaxNodeType::ASSIGNMENT) {
        add_compile_node_for_assignment(*expression);
      } else {
        add_compile_node_for_function_call(*expression);
      }
    }
  }
};
}  // namespace Compilation

#endif  // FUNCTIONSEXTRACTOR_H
