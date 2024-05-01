#ifndef COMPILETREENODES_H
#define COMPILETREENODES_H

#include <memory>
#include <string>
#include <vector>

#include "Instructions.h"

using std::vector, std::unique_ptr, std::string;

#define ACCEPT_COMPILER() \
  void accept(Compiler& compiler) override { compiler.compile(*this); }

#define COMPILE_NODE_TYPE(T) virtual void compile(const T&) = 0;

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

class Compiler {
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

  virtual ~Compiler() = default;
};

struct CompileNode {
  virtual void accept(Compiler&) = 0;

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

  ACCEPT_COMPILER();
};

struct FunctionDefinitionNode final : BaseFunctionDefinitionCompileNode {
  unique_ptr<CompileNode> body;

  FunctionDefinitionNode(string name, size_t arguments_count,
                         unique_ptr<CompileNode> body)
      : BaseFunctionDefinitionCompileNode(std::move(name), arguments_count),
        body(std::move(body)) {}

  ACCEPT_COMPILER();
};

struct ConstantNode final : CompileNode {
  ValueT value;

  explicit ConstantNode(ValueT value) : value(value) {}

  ACCEPT_COMPILER();
};

struct VariableNode final : CompileNode {
  size_t index;

  explicit VariableNode(size_t index) : index(index) {}

  ACCEPT_COMPILER();
};

struct RecursionParameterNode final : CompileNode {
  ACCEPT_COMPILER();
};

struct SelfCallNode final : CompileNode {
  ACCEPT_COMPILER();
};

struct InternalFunctionDefinitionNode final
    : BaseFunctionDefinitionCompileNode {
  using BaseFunctionDefinitionCompileNode::BaseFunctionDefinitionCompileNode;

  ACCEPT_COMPILER();
};

struct RecursiveFunctionDefinitionNode final
    : BaseFunctionDefinitionCompileNode {
  bool use_previous_value;
  unique_ptr<CompileNode> zero_case;
  unique_ptr<CompileNode> general_case;

  using BaseFunctionDefinitionCompileNode::BaseFunctionDefinitionCompileNode;

  ACCEPT_COMPILER();
};

struct FunctionCallNode final : CompileNode {
  size_t index;
  vector<unique_ptr<CompileNode>> arguments;

  ACCEPT_COMPILER();
};

struct ArgminCallNode final : CompileNode {
  unique_ptr<CompileNode> wrapped_call;

  ACCEPT_COMPILER();
};

struct AsteriskNode final : CompileNode {
  ACCEPT_COMPILER();
};
}  // namespace Compilation

#endif  // COMPILETREENODES_H
