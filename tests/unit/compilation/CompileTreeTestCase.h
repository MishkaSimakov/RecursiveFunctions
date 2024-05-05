#pragma once

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "RecursiveFunctions.h"

using Compilation::CompileTreeBuilder, Compilation::BytecodeCompiler,
    Compilation::BaseFunctionDefinitionCompileNode, Compilation::CompileNode,
    Preprocessing::Preprocessor, Preprocessing::TextSource,
    RecursiveFunctionsSyntax::RuleIdentifiers;
using std::string;

class CompileTreeTestCase : public ::testing::Test {
 protected:
  constexpr static auto kSuccessor = "successor";
  constexpr static auto kArgmin = "argmin";

  CompileTreeTestCase() { Logger::disable_category(Logger::ALL); }

  template <typename... Args>
    requires(std::is_constructible_v<string, Args> && ...)
  static std::unique_ptr<CompileNode> get_tree(Args... program) {
    auto preprocessor = Preprocessor();
    preprocessor.add_source<TextSource>("main",
                                        vector<string>{std::move(program)...});
    string program_text = preprocessor.process();

    auto tokens = LexicalAnalyzer::get_tokens(program_text);

    auto syntax_root =
        SyntaxTreeBuilder::build(tokens, RecursiveFunctionsSyntax::GetSyntax(),
                                 RuleIdentifiers::PROGRAM);

    CompileTreeBuilder ct_builder;
    for (auto& [name, args_count] : BytecodeCompiler::internal_functions) {
      ct_builder.add_internal_function(name, args_count);
    }

    return ct_builder.build(*syntax_root);
  }

  template <typename U, typename T>
    requires std::is_base_of_v<CompileNode, T> && std::is_base_of_v<T, U>
  static U& node_cast(const std::unique_ptr<T>& node) {
    return dynamic_cast<U&>(*node);
  }

  template <typename U, typename T>
    requires std::is_base_of_v<CompileNode, T> && std::is_base_of_v<T, U>
  static const U& node_cast(const T& node) {
    return dynamic_cast<const U&>(node);
  }

  template <typename T>
  static const BaseFunctionDefinitionCompileNode& get_function(
      const std::unique_ptr<T>& root, string function_name) {
    const auto& program = node_cast<Compilation::ProgramNode>(root);

    for (const auto& function : program.functions) {
      const auto& as_definition_node =
          node_cast<BaseFunctionDefinitionCompileNode>(function);

      if (as_definition_node.name == function_name) {
        return as_definition_node;
      }
    }

    throw std::runtime_error(fmt::format(
        "Requested function named {} was not found.", function_name));
  }

  template <typename T>
  static const BaseFunctionDefinitionCompileNode& get_function(
      const std::unique_ptr<T>& root, size_t index) {
    const auto& program = node_cast<Compilation::ProgramNode>(root);

    if (index >= program.functions.size()) {
      throw std::runtime_error(fmt::format(
          "Requested function with index {} was not found.", index));
    }

    return node_cast<BaseFunctionDefinitionCompileNode>(
        program.functions[index]);
  }
};
