#pragma once

#include <iostream>
#include <memory>
#include <stack>
#include <type_traits>

#include "compilation/CompileTreeNodes.h"
#include "intermediate_representation/BasicBlock.h"
#include "intermediate_representation/Function.h"
#include "intermediate_representation/Program.h"

namespace IR {
using std::vector, std::string, std::pair;
using namespace Compilation;

class IRCompiler final : public CompileTreeVisitor {
  Program program_;
  Function* current_function_{nullptr};
  BasicBlock* result_{nullptr};

  std::stack<FunctionCall> compiled_calls_stack_;

  size_t current_temporary_index_{0};
  Value recursion_parameter_temporary_{};
  Value asterisk_temporary_{};

  template <typename Callable>
  void wrap_with_function(std::string name, size_t arguments_count,
                          Callable&& lambda)
    requires std::is_invocable_v<Callable>
  {
    Function func(std::move(name));
    for (size_t i = 0; i < arguments_count; ++i) {
      func.arguments.emplace_back(i, ValueType::VIRTUAL_REGISTER);
    }

    current_temporary_index_ = arguments_count;
    current_function_ = &func;
    result_ = func.set_begin_block();

    lambda();

    func.finalize();
    program_.functions.push_back(std::move(func));
  }

  Value get_next_temporary() {
    return Value(current_temporary_index_++, ValueType::VIRTUAL_REGISTER);
  }

  void assign_or_pass_as_argument(Value value) {
    if (compiled_calls_stack_.empty()) {
      result_->instructions.push_back(std::make_unique<Return>(value));
      return;
    }

    auto& top_call = compiled_calls_stack_.top();

    if (top_call.name == ArgminOperatorNode::operator_name) {
      result_->instructions.push_back(std::make_unique<Branch>(value));
      return;
    }

    if (top_call.name == SuccessorOperatorNode::operator_name) {
      auto temp = get_next_temporary();
      result_->instructions.push_back(std::make_unique<Addition>(
          temp, value, Value(1, ValueType::CONSTANT)));

      compiled_calls_stack_.pop();
      assign_or_pass_as_argument(temp);

      return;
    }

    top_call.arguments.push_back(value);
  }

 public:
  Program get_ir(const CompileNode& root) {
    root.accept(*this);

    return std::move(program_);
  }

  void visit(const ProgramNode&) override;
  void visit(const FunctionDefinitionNode&) override;
  void visit(const RecursiveFunctionDefinitionNode&) override;
  void visit(const RecursionParameterNode&) override;
  void visit(const ArgminOperatorNode&) override;
  void visit(const SuccessorOperatorNode&) override;
  void visit(const ConstantNode&) override;
  void visit(const VariableNode&) override;
  void visit(const AsteriskNode&) override;
  void visit(const FunctionCallNode&) override;
  void visit(const SelfCallNode&) override;
  void visit(const ExternFunctionDefinitionNode&) override;
};
}  // namespace IR
