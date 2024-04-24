#ifndef COMPILETREEBUILDER_H
#define COMPILETREEBUILDER_H

#include <syntax/buffalo/SyntaxNode.h>

#include <memory>
#include <unordered_map>

#include "CompileTreeNodes.h"
#include "Compiler.h"

using std::unique_ptr, std::unordered_map, std::string, std::optional;

namespace Compilation {
class CompileTreeBuilder {
  unordered_map<string, size_t> functions_indices_;
  vector<unique_ptr<CompileNode>> functions_;
  unique_ptr<CompileNode> call_;

  constexpr static auto kArgminFunctionName = "argmin";

  struct ValueCompilationNodeBuilderParameters {
    bool is_inside_call_statement;
    bool is_inside_argmin_call;
    unordered_map<string, VariableInfo>* variables_map;

    static ValueCompilationNodeBuilderParameters for_call_statement() {
      return {true, false, nullptr};
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

  static FunctionType get_function_type(const SyntaxNode& info_node) {
    const auto& last_child = info_node.children.back();
    if (last_child->type == SyntaxNodeType::VARIABLE) {
      return FunctionType::STANDART;
    }

    if (last_child->value == "0") {
      return FunctionType::RECURSIVE_ZERO_CASE;
    }

    return FunctionType::RECURSIVE_GENERAL_CASE;
  }

  unique_ptr<CompileNode> build_constant_compile_node(
      const SyntaxNode& syntax_node) {
    size_t value = get_numeric_from_string(syntax_node);
    return std::make_unique<ConstantNode>(ValueT::construct_value(value));
  }

  unique_ptr<CompileNode> build_variable_compile_node(
      const SyntaxNode& syntax_node,
      const ValueCompilationNodeBuilderParameters& parameters) {
    if (!parameters.can_contain_variable()) {
      throw std::runtime_error(
          "Functions can not contain variables as arguments in this "
          "context.");
    }

    auto var_info = parameters.get_variable_info(syntax_node.value);
    if (var_info == nullptr) {
      throw std::runtime_error("Unknown variable inside function definition.");
    }

    var_info->was_used = true;

    switch (var_info->type) {
      case VariableType::VARIABLE:
        return std::make_unique<VariableNode>(var_info->id);
      case VariableType::RECURSION_PARAMETER:
        return std::make_unique<RecursionParameterNode>();
      case VariableType::RECURSION_CALL:
        return std::make_unique<SelfCallNode>();
      default:
        throw std::runtime_error("Unexpected syntax node type.");
    }
  }

  unique_ptr<CompileNode> build_argmin_call_compile_node(
      const SyntaxNode& syntax_node,
      ValueCompilationNodeBuilderParameters parameters) {
    if (syntax_node.children.size() != 1) {
      throw std::runtime_error("Argmin called with wrong count of arguments.");
    }

    const auto& wrapped_value = *syntax_node.children[0];

    auto node = std::make_unique<ArgminCallNode>();
    parameters.is_inside_argmin_call = true;

    node->wrapped_call = build_value_compile_node(wrapped_value, parameters);

    return node;
  }

  unique_ptr<CompileNode> build_function_call_compile_node(
      const SyntaxNode& syntax_node,
      const ValueCompilationNodeBuilderParameters& parameters) {
    const string& function_name = syntax_node.value;

    if (function_name == kArgminFunctionName) {
      return build_argmin_call_compile_node(syntax_node, parameters);
    }

    auto& children = syntax_node.children;

    auto function_index_itr = functions_indices_.find(function_name);
    if (function_index_itr == functions_indices_.end()) {
      throw std::runtime_error(string("Usage of function \"") + function_name +
                               "\" before definition.");
    }
    const auto& function_definition_node =
        static_cast<BaseFunctionDefinitionCompileNode&>(
            *functions_[function_index_itr->second]);

    auto arguments_count = function_definition_node.arguments_count;
    if (arguments_count != children.size()) {
      throw std::runtime_error(
          "Call of function with wrong number of arguments");
    }

    auto call_node = std::make_unique<FunctionCallNode>();
    call_node->index = function_index_itr->second;
    call_node->arguments.reserve(arguments_count);

    for (const auto& child : syntax_node.children) {
      call_node->arguments.push_back(
          build_value_compile_node(*child, parameters));
    }

    return call_node;
  }

  // value node is either: constant, variable, function call or argmin call
  unique_ptr<CompileNode> build_value_compile_node(
      const SyntaxNode& syntax_node,
      const ValueCompilationNodeBuilderParameters& parameters) {
    if (syntax_node.type == SyntaxNodeType::CONSTANT) {
      return build_constant_compile_node(syntax_node);
    }
    if (syntax_node.type == SyntaxNodeType::VARIABLE) {
      return build_variable_compile_node(syntax_node, parameters);
    }
    if (syntax_node.type == SyntaxNodeType::ASTERISK) {
      if (!parameters.can_contain_asterisk()) {
        throw std::runtime_error(
            "Asterisk may only be used inside argmin function call.");
      }

      return std::make_unique<AsteriskNode>();
    }

    return build_function_call_compile_node(syntax_node, parameters);
  }

  void visit_call_statement(const SyntaxNode& syntax_node) {
    if (call_ != nullptr) {
      throw std::runtime_error("There must be only one call in program.");
    }

    auto params = ValueCompilationNodeBuilderParameters::for_call_statement();
    call_ = build_value_compile_node(syntax_node, params);
  }

  template <typename T>
    requires std::is_base_of_v<BaseFunctionDefinitionCompileNode, T>
  T& construct_definition_node(auto&&... args) {
    auto node_ptr = std::make_unique<T>(std::forward<decltype(args)>(args)...);
    functions_indices_[node_ptr->name] = functions_.size();

    auto& result = *node_ptr;
    functions_.push_back(std::move(node_ptr));

    return result;
  }

  void visit_assignment_statement(const SyntaxNode& syntax_node) {
    unordered_map<string, VariableInfo> variables;

    const auto& info_node = *syntax_node.children[0];
    const auto& value_node = *syntax_node.children[1];

    size_t variables_count = info_node.children.size();
    const string& function_name = info_node.value;

    auto def_compile_node_itr = functions_indices_.find(function_name);
    bool function_was_found = def_compile_node_itr != functions_indices_.end();

    auto function_type = get_function_type(info_node);
    bool is_recursive = function_type != FunctionType::STANDART;

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

    ValueCompilationNodeBuilderParameters parameters;
    parameters.variables_map = &variables;
    parameters.is_inside_argmin_call = false;
    parameters.is_inside_call_statement = false;

    if (function_type == FunctionType::RECURSIVE_GENERAL_CASE) {
      auto& last_varname = info_node.children.back()->value;
      variables[last_varname].type = VariableType::RECURSION_PARAMETER;
      variables[function_name].type = VariableType::RECURSION_CALL;
    }

    auto value_compile_node = build_value_compile_node(value_node, parameters);

    if (is_recursive) {
      complete_recursive_function_definition(
          function_name, variables_count, variables,
          std::move(value_compile_node), function_type);
      return;
    }

    construct_definition_node<FunctionDefinitionNode>(
        function_name, variables_count, std::move(value_compile_node));
  }

  RecursiveFunctionDefinitionNode& get_recursive_fucntion_definition_node(
      const string& name, size_t arguments_count) {
    if (!functions_indices_.contains(name)) {
      return construct_definition_node<RecursiveFunctionDefinitionNode>(
          name, arguments_count);
    }

    try {
      return dynamic_cast<RecursiveFunctionDefinitionNode&>(
          *functions_[functions_indices_[name]]);
    } catch (std::bad_cast) {
      throw std::runtime_error("Redefinition of non-recursive function");
    }
  }

  void complete_recursive_function_definition(
      const string& name, size_t arguments_count,
      const unordered_map<string, VariableInfo>& variables,
      std::unique_ptr<CompileNode> value_compile_node,
      FunctionType function_type) {
    auto& node = get_recursive_fucntion_definition_node(name, arguments_count);

    if (node.arguments_count != arguments_count) {
      throw std::runtime_error(
          "Recursive function cases defined with different number of "
          "arguments");
    }

    if (function_type == FunctionType::RECURSIVE_GENERAL_CASE) {
      node.use_previous_value = variables.at(name).was_used;
    }

    if (function_type == FunctionType::RECURSIVE_ZERO_CASE) {
      if (node.zero_case != nullptr) {
        throw std::runtime_error(
            "Redefinition of recursive function zero case");
      }

      node.zero_case = std::move(value_compile_node);
    } else {
      if (node.general_case != nullptr) {
        throw std::runtime_error(
            "Redefinition of recursive function general case");
      }

      node.general_case = std::move(value_compile_node);
    }
  }

 public:
  void add_internal_function(string name, size_t arguments_count) {
    construct_definition_node<InternalFunctionDefinitionNode>(name,
                                                              arguments_count);
  }

  unique_ptr<ProgramNode> build(const SyntaxNode& syntax_tree_root) {
    for (const auto& statement : syntax_tree_root.children) {
      // there are two types of statements now: assignment and call
      if (statement->type == SyntaxNodeType::ASSIGNMENT) {
        visit_assignment_statement(*statement);
      } else {
        visit_call_statement(*statement);
      }
    }

    for (auto& function : functions_) {
      auto recursive_ptr =
          dynamic_cast<RecursiveFunctionDefinitionNode*>(function.get());

      if (recursive_ptr == nullptr) {
        continue;
      }

      if (recursive_ptr->zero_case == nullptr ||
          recursive_ptr->general_case == nullptr) {
        throw std::runtime_error(
            "Recursive function definition is not complete");
      }
    }

    if (call_ == nullptr) {
      throw std::runtime_error("Program must have function call");
    }

    auto program_node = std::make_unique<ProgramNode>();
    program_node->functions = std::move(functions_);
    program_node->call = std::move(call_);

    return program_node;
  }
};
}  // namespace Compilation

#endif  // COMPILETREEBUILDER_H
