#pragma once

#include <stack>

#include "compilation/CompileTreeNodes.h"
#include "intermediate_representation/BasicBlock.h"

namespace IR {
using std::vector, std::string, std::pair;
using namespace Compilation;

class IRCompiler final : public CompileTreeVisitor {
  Program program_;
  BasicBlock result_{};

  std::stack<FunctionCall> compiled_calls_stack_;

  size_t current_argument_index_{0};
  size_t current_temporary_index_{0};

  BasicBlock compile_node(const std::unique_ptr<CompileNode>& node) {
    node->accept(*this);

    auto result = std::move(result_);
    result_ = BasicBlock();

    return result;
  }

  Temporary get_next_temporary() {
    return Temporary(current_temporary_index_++);
  }

 public:
  static const vector<pair<string, size_t>> internal_functions;

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
