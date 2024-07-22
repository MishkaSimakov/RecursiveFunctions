#pragma once

#include <memory>
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
  BasicBlock* result_{nullptr};

  std::stack<FunctionCall> compiled_calls_stack_;

  size_t current_temporary_index_{0};
  Temporary recursion_parameter_temporary_{};
  Temporary asterisk_temporary_{};

  template <typename Callable>
  void wrap_with_function(std::string name, size_t arguments_count,
                          Callable&& lambda)
    requires std::is_invocable_v<Callable>
  {
    Function func(std::move(name));
    func.arguments_count = arguments_count;

    current_temporary_index_ = arguments_count;
    current_function_ = &func;
    result_ = func.set_begin_block();

    lambda();

    func.finalize();
    program_.functions.push_back(std::move(func));
  }

  Temporary get_next_temporary() {
    return Temporary{current_temporary_index_++};
  }

  void assign_or_pass_as_argument(TemporaryOrConstant value) {
    if (compiled_calls_stack_.empty()) {
      auto return_instruction = std::make_unique<Return>();
      return_instruction->value = value;
      result_->instructions.push_back(std::move(return_instruction));

      return;
    }

    // TODO: replace argmin with constant
    if (compiled_calls_stack_.top().name == "argmin") {
      auto branch_instruction = std::make_unique<Branch>();
      branch_instruction->value = value;
      result_->instructions.push_back(std::move(branch_instruction));

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
