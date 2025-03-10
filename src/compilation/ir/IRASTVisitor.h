#pragma once
#include "ast/ASTVisitor.h"
#include "compilation/GlobalContext.h"
#include "compilation/ModuleContext.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

namespace Front {
class IRASTVisitor : public ASTVisitor<IRASTVisitor, true, Order::POSTORDER> {
  llvm::LLVMContext& llvm_context_;
  std::unique_ptr<llvm::Module> llvm_module_;
  std::unique_ptr<llvm::IRBuilder<>> llvm_ir_builder_;

  ModuleContext& module_;

  llvm::Value* current_expr_value_{nullptr};

  // compiler must not pass through some kind of nodes. If it does, then I've
  // made some mistake developing it
  [[noreturn]] static void unreachable_node() {
    unreachable("Compiler doesn't pass through this kind of nodes.");
  }

  llvm::Type* map_type(Type* type) const;

  llvm::Value* compile_expression(Expression* expr);

 public:
  IRASTVisitor(llvm::LLVMContext& llvm_context, ModuleContext& module)
      : llvm_context_(llvm_context),
        llvm_module_(
            std::make_unique<llvm::Module>(module.name, llvm_context_)),
        llvm_ir_builder_(std::make_unique<llvm::IRBuilder<>>(llvm_context_)),
        module_(module) {}

  bool traverse_function_declaration(const FunctionDecl& value);
  bool traverse_return_statement(const ReturnStmt& value);
  bool visit_integer_literal(const IntegerLiteral& value);

  std::unique_ptr<llvm::Module> compile();
};
}  // namespace Front
