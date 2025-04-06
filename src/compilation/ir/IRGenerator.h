#pragma once
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#include "ast/ASTVisitor.h"
#include "compilation/ModuleContext.h"
#include "compilation/mangling/Mangler.h"

namespace Front {
struct IRGeneratorConfig : ASTVisitorConfig {
  static constexpr auto order() { return Order::POSTORDER; }
  static constexpr auto is_const() { return true; }
  static constexpr auto override_all() { return true; }
};

struct IRContext {
  llvm::Module& llvm_module;

  Mangler& mangler;
  StringPool& strings;
  TypesStorage& types;

  llvm::LLVMContext& get_llvm_context() { return llvm_module.getContext(); }
};

class IRGenerator : public ASTVisitor<IRGenerator, IRGeneratorConfig> {
  llvm::LLVMContext& llvm_context_;
  std::unique_ptr<llvm::Module> llvm_module_;
  std::unique_ptr<llvm::IRBuilder<>> llvm_ir_builder_;

  Mangler mangler_;

  // information about current function
  struct LocalVariableInfo {
    bool has_indirection{false};
    llvm::Type* original_type{nullptr};
    llvm::Value* pointer{nullptr};
  };
  std::unordered_map<const Declaration*, LocalVariableInfo> local_variables_;
  llvm::BasicBlock* alloca_block_{nullptr};

  llvm::Value* current_initializing_value_{nullptr};

  ModuleContext& module_;

  llvm::Value* current_expr_value_{nullptr};

  // compiler must not pass through some kind of nodes. If it does, then I've
  // made some mistake developing it
  [[noreturn]] static void unreachable_node() {
    unreachable("Compiler doesn't pass through this kind of nodes.");
  }

  llvm::Type* map_type(Type* type) const;
  llvm::Value* compile_expression(const Expression& expr);
  llvm::Value* compile_initializer_for(llvm::Value* var_ptr,
                                       const Expression& expr);

  void store_argument_value(const VariableDecl& argument, llvm::Value* value);
  void create_function_arguments(const FunctionSymbolInfo& info,
                                 llvm::Function* llvm_fun);
  llvm::Value* get_local_variable_value(const VariableDecl& decl);
  llvm::Function* get_or_insert_function(const FunctionSymbolInfo& info);
  std::unique_ptr<llvm::IRBuilder<>> get_alloca_builder();

  void emit_store(Expression& source, llvm::Value* destination);

  IRContext get_context();

 public:
  IRGenerator(llvm::LLVMContext& llvm_context, ModuleContext& module);

  // statements
  bool traverse_if_statement(const IfStmt& value);
  bool traverse_while_statement(const WhileStmt& value);
  bool traverse_return_statement(const ReturnStmt& value);
  bool traverse_assignment_statement(const AssignmentStmt& value);

  // declarations
  bool traverse_variable_declaration(const VariableDecl& value);
  bool traverse_function_declaration(const FunctionDecl& value);

  // expressions
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
  bool traverse_implicit_tuple_copy_expression(
      const ImplicitTupleCopyExpr& value);
  bool traverse_unary_operator(const UnaryOperator& value);

  std::unique_ptr<llvm::Module> compile();
};
}  // namespace Front
