#include "compilation/CompileTreeBuilder.h"

#include <fmt/format.h>

#include "compilation/CompileTreeNodes.h"
#include "utils/Constants.h"

using Compilation::CompileTreeBuilder, Compilation::CompileNode,
    Compilation::RecursiveFunctionDefinitionNode, Compilation::ProgramNode;

CompileTreeBuilder::FunctionType CompileTreeBuilder::get_function_type(
    const SyntaxNode& info_node) {
  if (info_node.children.empty()) {
    return FunctionType::STANDART;
  }

  const auto& last_child = info_node.children.back();
  if (last_child->type == SyntaxNodeType::VARIABLE) {
    return FunctionType::STANDART;
  }

  if (last_child->value == "0") {
    return FunctionType::RECURSIVE_ZERO_CASE;
  }

  return FunctionType::RECURSIVE_GENERAL_CASE;
}

unique_ptr<CompileNode> CompileTreeBuilder::build_constant_compile_node(
    const SyntaxNode& syntax_node) {
  size_t value = get_numeric_from_string(syntax_node);
  return std::make_unique<ConstantNode>(value);
}

unique_ptr<CompileNode> CompileTreeBuilder::build_variable_compile_node(
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
      return std::make_unique<RecursionParameterNode>(var_info->id);
    case VariableType::RECURSION_CALL:
      return [&parameters] {
        auto self_call = std::make_unique<SelfCallNode>();
        self_call->arguments_count =
            parameters.current_function_definition->children.size();
        self_call->name = parameters.current_function_definition->value;

        return self_call;
      }();

    default:
      throw std::runtime_error("Unexpected syntax node type.");
  }
}

unique_ptr<CompileNode> CompileTreeBuilder::build_argmin_operator_compile_node(
    const SyntaxNode& syntax_node,
    ValueCompilationNodeBuilderParameters parameters) {
  if (parameters.is_inside_argmin_call) {
    throw std::runtime_error("Argmin cannot be used inside argmin.");
  }

  if (syntax_node.children.size() != 1) {
    throw std::runtime_error(
        "Argmin operator used with wrong count of arguments.");
  }

  const auto& wrapped_value = *syntax_node.children[0];

  auto node = std::make_unique<ArgminOperatorNode>();
  parameters.is_inside_argmin_call = true;

  node->wrapped = build_value_compile_node(wrapped_value, parameters);

  return node;
}

unique_ptr<CompileNode>
CompileTreeBuilder::build_successor_operator_compile_node(
    const SyntaxNode& syntax_node,
    const ValueCompilationNodeBuilderParameters& parameters) {
  if (syntax_node.children.size() != 1) {
    throw std::runtime_error(
        "Successor operator used with wrong count of arguments.");
  }

  const auto& wrapped_value = *syntax_node.children[0];

  auto node = std::make_unique<SuccessorOperatorNode>();
  node->wrapped = build_value_compile_node(wrapped_value, parameters);

  return node;
}

unique_ptr<CompileNode> CompileTreeBuilder::build_function_call_compile_node(
    const SyntaxNode& syntax_node,
    const ValueCompilationNodeBuilderParameters& parameters) {
  const string& function_name = syntax_node.value;

  if (function_name == ArgminOperatorNode::operator_name) {
    return build_argmin_operator_compile_node(syntax_node, parameters);
  }
  if (function_name == SuccessorOperatorNode::operator_name) {
    return build_successor_operator_compile_node(syntax_node, parameters);
  }

  auto& children = syntax_node.children;

  auto function_index_itr = functions_indices_.find(function_name);
  if (function_index_itr == functions_indices_.end()) {
    throw std::runtime_error(string("Usage of function \"") + function_name +
                             "\" before definition.");
  }
  const auto& function_definition_node = static_cast<BaseFunctionDeclaration&>(
      *functions_[function_index_itr->second]);

  auto arguments_count = function_definition_node.arguments_count;
  if (arguments_count != children.size()) {
    throw std::runtime_error("Call of function with wrong number of arguments");
  }

  auto call_node = std::make_unique<FunctionCallNode>();
  call_node->index = function_index_itr->second;
  call_node->name = function_name;
  call_node->arguments.reserve(arguments_count);

  for (const auto& child : syntax_node.children) {
    call_node->arguments.push_back(
        build_value_compile_node(*child, parameters));
  }

  return call_node;
}

unique_ptr<CompileNode> CompileTreeBuilder::build_value_compile_node(
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

void CompileTreeBuilder::visit_declaration(const SyntaxNode& syntax_node) {
  const SyntaxNode& function_node = *syntax_node.children[0];

  auto& name = function_node.value;

  auto itr = functions_indices_.find(name);

  if (itr != functions_indices_.end()) {
    throw std::runtime_error("Function defined or declared twice.");
  }

  construct_function_node<ExternFunctionDefinitionNode>(
      name, function_node.children.size());
}

void CompileTreeBuilder::visit_assignment_statement(
    const SyntaxNode& syntax_node) {
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
      throw std::runtime_error("All function variables names must be unique.");
    }

    // TODO: changed order of variables (before in info indices were stored in
    // reversed order)
    variables.emplace(std::move(varname),
                      VariableInfo{i, VariableType::VARIABLE});
  }

  ValueCompilationNodeBuilderParameters parameters;
  parameters.variables_map = &variables;
  parameters.is_inside_argmin_call = false;
  parameters.is_inside_call_statement = false;
  parameters.current_function_definition = &info_node;

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

  construct_function_node<FunctionDefinitionNode>(
      function_name, variables_count, std::move(value_compile_node));
}

RecursiveFunctionDefinitionNode&
CompileTreeBuilder::get_recursive_function_definition_node(
    const string& name, size_t arguments_count) {
  if (!functions_indices_.contains(name)) {
    return construct_function_node<RecursiveFunctionDefinitionNode>(
        name, arguments_count);
  }

  try {
    return dynamic_cast<RecursiveFunctionDefinitionNode&>(
        *functions_[functions_indices_[name]]);
  } catch (std::bad_cast) {
    throw std::runtime_error("Redefinition of non-recursive function");
  }
}

void CompileTreeBuilder::complete_recursive_function_definition(
    const string& name, size_t arguments_count,
    const unordered_map<string, VariableInfo>& variables,
    std::unique_ptr<CompileNode> value_compile_node,
    FunctionType function_type) {
  auto& node = get_recursive_function_definition_node(name, arguments_count);

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
      throw std::runtime_error("Redefinition of recursive function zero case");
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

unique_ptr<ProgramNode> CompileTreeBuilder::build(
    const SyntaxNode& syntax_tree_root) {
  for (const auto& statement : syntax_tree_root.children) {
    // there are two types of statements now: function definition and extern
    // declaration
    if (statement->type == SyntaxNodeType::ASSIGNMENT) {
      // it is definition
      visit_assignment_statement(*statement);
    } else {
      // it is extern function declaration
      visit_declaration(*statement);
    }
  }

  bool found_main_function = false;
  for (auto& node : functions_) {
    auto& function = static_cast<const BaseFunctionDeclaration&>(*node);

    // there is upper bound on arguments count for now
    if (function.arguments_count > Constants::max_arguments) {
      throw std::runtime_error(fmt::format("Maximum {} arguments in function.",
                                           Constants::max_arguments));
    }

    // main function must exist and have zero arguments
    if (function.name == Constants::entrypoint) {
      found_main_function = true;

      if (function.arguments_count != 0) {
        throw std::runtime_error(
            fmt::format("Function \"{}\" must have zero arguments.",
                        Constants::entrypoint));
      }
    }

    auto recursive_ptr =
        dynamic_cast<const RecursiveFunctionDefinitionNode*>(&function);

    if (recursive_ptr == nullptr) {
      continue;
    }

    if (recursive_ptr->zero_case == nullptr ||
        recursive_ptr->general_case == nullptr) {
      throw std::runtime_error("Recursive function definition is not complete");
    }
  }

  if (!found_main_function) {
    throw std::runtime_error(fmt::format(
        "Program must contain function called \"{}\"", Constants::entrypoint));
  }

  auto program_node = std::make_unique<ProgramNode>();
  program_node->functions = std::move(functions_);

  return program_node;
}
