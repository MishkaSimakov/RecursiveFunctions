#pragma once

#include <memory>
#include <optional>
#include <unordered_map>

#include "CompileTreeNodes.h"
#include "syntax/buffalo/SyntaxNode.h"

using std::unique_ptr, std::unordered_map, std::string, std::optional;

namespace Compilation {
enum class VariableType { VARIABLE, RECURSION_PARAMETER, RECURSION_CALL };

struct VariableInfo {
  size_t id;
  VariableType type;
  bool was_used = false;
};

class CompileTreeBuilder {
  unordered_map<string, size_t> functions_indices_;
  vector<unique_ptr<CompileNode>> functions_;

  struct ValueCompilationNodeBuilderParameters {
    const SyntaxNode* current_function_definition;
    bool is_inside_call_statement;
    bool is_inside_argmin_call;
    unordered_map<string, VariableInfo>* variables_map;

    static ValueCompilationNodeBuilderParameters for_call_statement() {
      return {nullptr, true, false, nullptr};
    }

    bool can_contain_variable() const { return !is_inside_call_statement; }

    bool can_contain_asterisk() const { return is_inside_argmin_call; }

    VariableInfo* get_variable_info(const string& variable_name) const {
      if (variables_map == nullptr) {
        return nullptr;
      }

      const auto itr = variables_map->find(variable_name);

      if (itr == variables_map->end()) {
        return nullptr;
      }

      return &itr->second;
    }
  };

  static size_t get_numeric_from_string(const SyntaxNode& node) {
    return std::stoi(node.value);
  }

  enum class FunctionType {
    STANDART,
    RECURSIVE_ZERO_CASE,
    RECURSIVE_GENERAL_CASE
  };

  static FunctionType get_function_type(const SyntaxNode& info_node);

  unique_ptr<CompileNode> build_constant_compile_node(
      const SyntaxNode& syntax_node);

  unique_ptr<CompileNode> build_variable_compile_node(
      const SyntaxNode& syntax_node,
      const ValueCompilationNodeBuilderParameters& parameters);

  unique_ptr<CompileNode> build_argmin_operator_compile_node(
      const SyntaxNode& syntax_node,
      ValueCompilationNodeBuilderParameters parameters);

  unique_ptr<CompileNode> build_successor_operator_compile_node(
      const SyntaxNode& syntax_node,
      const ValueCompilationNodeBuilderParameters& parameters);

  unique_ptr<CompileNode> build_function_call_compile_node(
      const SyntaxNode& syntax_node,
      const ValueCompilationNodeBuilderParameters& parameters);

  // value node is either: constant, variable, function call or argmin call
  unique_ptr<CompileNode> build_value_compile_node(
      const SyntaxNode& syntax_node,
      const ValueCompilationNodeBuilderParameters& parameters);

  void visit_call_statement(const SyntaxNode& syntax_node);

  void visit_declaration(const SyntaxNode& syntax_node);

  template <typename T>
    requires std::is_base_of_v<BaseFunctionDeclaration, T>
  T& construct_function_node(auto&&... args) {
    auto node_ptr = std::make_unique<T>(std::forward<decltype(args)>(args)...);
    functions_indices_[node_ptr->name] = functions_.size();

    auto& result = *node_ptr;
    functions_.push_back(std::move(node_ptr));

    return result;
  }

  void visit_assignment_statement(const SyntaxNode& syntax_node);

  RecursiveFunctionDefinitionNode& get_recursive_function_definition_node(
      const string& name, size_t arguments_count);

  void complete_recursive_function_definition(
      const string& name, size_t arguments_count,
      const unordered_map<string, VariableInfo>& variables,
      std::unique_ptr<CompileNode> value_compile_node,
      FunctionType function_type);

 public:
  unique_ptr<ProgramNode> build(const SyntaxNode& syntax_tree_root);
};
}  // namespace Compilation
