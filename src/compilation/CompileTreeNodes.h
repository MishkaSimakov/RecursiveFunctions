#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Instructions.h"

using std::vector, std::unique_ptr, std::string;

#define ACCEPT_VISITOR()                                     \
  void accept(CompileTreeVisitor& compiler) const override { \
    compiler.visit(*this);                                   \
  }

#define COMPILE_NODE_TYPE(T) virtual void visit(const T&){};

namespace Compilation {
struct ProgramNode;
struct FunctionDefinitionNode;
struct RecursiveFunctionDefinitionNode;
struct RecursionParameterNode;
struct ArgminCallNode;
struct InternalFunctionDefinitionNode;
struct ConstantNode;
struct VariableNode;
struct AsteriskNode;
struct FunctionCallNode;
struct SelfCallNode;

class CompileTreeVisitor {
 public:
  COMPILE_NODE_TYPE(ProgramNode);
  COMPILE_NODE_TYPE(FunctionDefinitionNode);
  COMPILE_NODE_TYPE(RecursiveFunctionDefinitionNode);
  COMPILE_NODE_TYPE(RecursionParameterNode);
  COMPILE_NODE_TYPE(ArgminCallNode);
  COMPILE_NODE_TYPE(InternalFunctionDefinitionNode);
  COMPILE_NODE_TYPE(ConstantNode);
  COMPILE_NODE_TYPE(VariableNode);
  COMPILE_NODE_TYPE(AsteriskNode);
  COMPILE_NODE_TYPE(FunctionCallNode);
  COMPILE_NODE_TYPE(SelfCallNode);

  virtual ~CompileTreeVisitor() = default;
};

struct CompileNode {
  virtual void accept(CompileTreeVisitor&) const = 0;

  virtual ~CompileNode() {}
};

struct BaseFunctionDefinitionCompileNode : CompileNode {
  string name;
  size_t arguments_count;

  BaseFunctionDefinitionCompileNode(string name, size_t arguments_count)
      : name(std::move(name)), arguments_count(arguments_count) {}
};

struct ProgramNode final : CompileNode {
  vector<unique_ptr<CompileNode>> functions;
  unique_ptr<CompileNode> call;

  ACCEPT_VISITOR();
};

struct FunctionDefinitionNode final : BaseFunctionDefinitionCompileNode {
  unique_ptr<CompileNode> body;

  FunctionDefinitionNode(string name, size_t arguments_count,
                         unique_ptr<CompileNode> body)
      : BaseFunctionDefinitionCompileNode(std::move(name), arguments_count),
        body(std::move(body)) {}

  ACCEPT_VISITOR();
};

struct ConstantNode final : CompileNode {
  ssize_t value;

  explicit ConstantNode(ssize_t value) : value(value) {}

  ACCEPT_VISITOR();
};

struct VariableNode : CompileNode {
  size_t index;

  explicit VariableNode(size_t index) : index(index) {}

  ACCEPT_VISITOR();
};

struct RecursionParameterNode final : VariableNode {
  using VariableNode::VariableNode;

  ACCEPT_VISITOR();
};

struct InternalFunctionDefinitionNode final
    : BaseFunctionDefinitionCompileNode {
  using BaseFunctionDefinitionCompileNode::BaseFunctionDefinitionCompileNode;

  ACCEPT_VISITOR();
};

struct RecursiveFunctionDefinitionNode final
    : BaseFunctionDefinitionCompileNode {
  bool use_previous_value;
  unique_ptr<CompileNode> zero_case;
  unique_ptr<CompileNode> general_case;

  using BaseFunctionDefinitionCompileNode::BaseFunctionDefinitionCompileNode;

  size_t get_recursion_parameter_index() const { return arguments_count - 1; }

  ACCEPT_VISITOR();
};

struct FunctionCallNode final : CompileNode {
  size_t index;
  vector<unique_ptr<CompileNode>> arguments;

  string name;

  ACCEPT_VISITOR();
};

struct SelfCallNode final : CompileNode {
  size_t arguments_count;
  string name;

  ACCEPT_VISITOR();
};

struct ArgminCallNode final : CompileNode {
  unique_ptr<CompileNode> wrapped_call;

  ACCEPT_VISITOR();
};

struct AsteriskNode final : CompileNode {
  ACCEPT_VISITOR();
};
}  // namespace Compilation
