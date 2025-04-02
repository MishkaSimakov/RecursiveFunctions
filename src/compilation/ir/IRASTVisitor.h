#pragma once
#include "ast/ASTVisitor.h"
#include "compilation/GlobalContext.h"
#include "compilation/ModuleContext.h"
#include "compilation/mangling/Mangler.h"
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
struct IRASTVisitorConfig : ASTVisitorConfig {
  static constexpr auto order() { return Order::POSTORDER; }
  static constexpr auto is_const() { return true; }
  static constexpr auto override_all() { return true; }
};

class IRASTVisitor : public ASTVisitor<IRASTVisitor, IRASTVisitorConfig> {
  llvm::LLVMContext& llvm_context_;
  std::unique_ptr<llvm::Module> llvm_module_;
  std::unique_ptr<llvm::IRBuilder<>> llvm_ir_builder_;

  Mangler mangler_;

  std::unordered_map<const Declaration*, llvm::Value*> identifiers_addresses_;

  ModuleContext& module_;

  llvm::Value* current_expr_value_{nullptr};

  // compiler must not pass through some kind of nodes. If it does, then I've
  // made some mistake developing it
  [[noreturn]] static void unreachable_node() {
    unreachable("Compiler doesn't pass through this kind of nodes.");
  }

  llvm::Type* map_type(Type* type) const;
  llvm::Value* compile_expression(const Expression& expr);

  llvm::Function* get_or_insert_function(const FunctionSymbolInfo& info);

  void allocate_local_variables(const FunctionSymbolInfo& info);

 public:
  IRASTVisitor(llvm::LLVMContext& llvm_context, ModuleContext& module)
      : llvm_context_(llvm_context),
        llvm_module_(
            std::make_unique<llvm::Module>(module.name, llvm_context_)),
        llvm_ir_builder_(std::make_unique<llvm::IRBuilder<>>(llvm_context_)),
        mangler_(module.get_strings_pool()),
        module_(module) {}

  bool traverse_if_statement(const IfStmt& value);
  bool traverse_while_statement(const WhileStmt& value);
  bool traverse_return_statement(const ReturnStmt& value);
  bool traverse_assignment_statement(const AssignmentStmt& value);

  bool traverse_variable_declaration(const VariableDecl& value);
  bool traverse_function_declaration(const FunctionDecl& value);

  bool traverse_implicit_lvalue_to_rvalue_conversion_expression(
      const ImplicitLvalueToRvalueConversionExpr& value);
  bool traverse_call_expression(const CallExpr& value);
  bool traverse_id_expression(const IdExpr& value);
  bool traverse_binary_operator(const BinaryOperator& value);
  bool visit_integer_literal(const IntegerLiteral& value);
  bool visit_bool_literal(const BoolLiteral& value);
  bool traverse_member_expression(const MemberExpr& value);
  bool traverse_tuple_expression(const TupleExpr& value);
  bool traverse_tuple_index_expression(const TupleIndexExpr& value);

  std::unique_ptr<llvm::Module> compile();
};
}  // namespace Front
