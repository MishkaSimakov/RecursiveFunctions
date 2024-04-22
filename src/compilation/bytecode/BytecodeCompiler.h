#ifndef BYTECODECOMPILER_H
#define BYTECODECOMPILER_H
#include "compilation/CompileTreeNodes.h"

namespace Compilation {
class BytecodeCompiler final : public Compiler {
  vector<list<Instruction>> compiled_functions_;
  size_t current_offset_{};

  list<Instruction> result_;

  list<Instruction> compile_node(const std::unique_ptr<CompileNode>& node) {
    node->accept(*this);
    auto result = std::move(result_);
    result_.clear();

    return result;
  }

 public:
  vector<Instruction> get_result() const;

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
};
}  // namespace Compilation

#endif  // BYTECODECOMPILER_H
