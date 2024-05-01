#ifndef CPPCOMPILER_H
#define CPPCOMPILER_H
#include <regex>

#include "compilation/CompileTreeNodes.h"

namespace Compilation {
class CppCompiler final : Compiler {
  string result_;
  string current_function_name;

  static string format(string format, auto&&... args) {
    auto args_vector = {args...};
  }

  string compile_node(const std::unique_ptr<CompileNode>& node) {
    node->accept(*this);

    auto result = std::move(result_);
    result_.clear();

    return result;
  }

  static string get_variable_name(size_t index) {
    return string("variable_") + std::to_string(index);
  }

  static string get_asterisk_name() { return "asterisk"; }

  static string get_recursion_parameter_name() { return "recursion"; }

 public:
  string get_result() const { return result_; }

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
}  // namespace Compilation

#endif  // CPPCOMPILER_H
