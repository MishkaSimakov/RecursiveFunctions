#pragma once

#include <stack>
#include <type_traits>

#include "compilation/CompileTreeNodes.h"
#include "intermediate_representation/BasicBlock.h"

namespace IR {
using std::vector, std::string, std::pair;
using namespace Compilation;

class IRCompiler final : public CompileTreeVisitor {
  Program program_;
  Function* current_function_{nullptr};
  BasicBlock result_;

  std::stack<FunctionCall> compiled_calls_stack_;

  size_t current_temporary_index_{0};
  Temporary recursion_parameter_temporary_{};
  Temporary asterisk_temporary_{};

  BasicBlock compile_node(const std::unique_ptr<CompileNode>& node) {
    node->accept(*this);

    auto result = std::move(result_);
    result_ = BasicBlock();

    return result;
  }

  template <typename Callable>
  void wrap_with_function(std::string name, size_t start_temp_index, Callable&& lambda)
    requires std::is_invocable_v<Callable> {
    Function func(std::move(name));

    current_temporary_index_ = start_temp_index;
    current_function_ = &func;

    lambda();

    current_function_ = nullptr;

    program_.functions.push_back(std::move(func));
  }

  Temporary get_next_temporary() {
    return Temporary{current_temporary_index_++};
  }

  void assign_or_pass_as_argument(TemporaryOrConstant value) {
    if (compiled_calls_stack_.empty()) {
      // we should return variable immediately
      result_.end_value = value;
      return;
    }

    auto& function_call = compiled_calls_stack_.top();
    function_call.arguments.push_back(value);
  }

 public:
  static const vector<pair<string, size_t>> internal_functions;

  Program get_ir(const CompileNode& root) {
    root.accept(*this);

    return std::move(program_);
  }

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
}  // namespace IR
