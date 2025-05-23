#pragma once
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include "Context.h"
#include "Function.h"
#include "TypesMapper.h"
#include "ast/ASTVisitor.h"
#include "compilation/ModuleContext.h"
#include "compilation/mangling/Mangler.h"

namespace Front {
struct IRGeneratorConfig : ASTVisitorConfig {
  static constexpr auto order() { return Order::POSTORDER; }
  static constexpr auto is_const() { return true; }
  static constexpr auto override_all() { return true; }
};

struct Value {
  llvm::Value* llvm_value;
  bool has_indirection;

  Value(llvm::Value* llvm_value)
      : llvm_value(llvm_value), has_indirection(false) {}

  Value() : llvm_value(nullptr), has_indirection(false) {}

  bool operator==(const Value& other) const {
    return llvm_value == other.llvm_value;
  }

  static Value invalid() { return Value(); }
};

class IRGenerator : public ASTVisitor<IRGenerator, IRGeneratorConfig> {
  llvm::LLVMContext& llvm_context_;
  std::unique_ptr<llvm::Module> llvm_module_;
  std::unique_ptr<llvm::IRBuilder<>> llvm_ir_builder_;

  ModuleContext& module_;

  Mangler mangler_;
  TypesMapper types_mapper_;

  // information about current function
  std::unordered_map<const Declaration*, llvm::Value*> local_variables_;

  std::optional<IRFunction> current_function_{std::nullopt};

  Value current_expr_value_{Value::invalid()};

  // compiler must not pass through some kind of nodes. If it does, then I've
  // made some mistake developing it
  [[noreturn]] static void unreachable_node() {
    unreachable("Compiler doesn't pass through this kind of nodes.");
  }

  Value compile_expr(const std::unique_ptr<Expression>& expr);

  void create_function_arguments();
  llvm::Value* get_local_variable_value(const VariableDecl& decl);
  IRFunctionDecl get_or_insert_function(const FunctionSymbolInfo& info);
  std::unique_ptr<llvm::IRBuilder<>> get_alloca_builder();

  llvm::Value* get_slot(Type* type);

  IRContext get_context();

  void create_store(llvm::Value* destination, Value source, Type* source_type);
  void create_tuple_copy_constructor(llvm::Value* destination, Value source,
                                     TupleType* source_type);
  Value remove_indirection(Value value, Type* type);

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
  bool visit_string_literal(const StringLiteral& value);
  bool traverse_member_expression(const MemberExpr& value);
  bool traverse_tuple_expression(const TupleExpr& value);
  bool traverse_tuple_index_expression(const TupleIndexExpr& value);
  bool traverse_implicit_tuple_copy_expression(
      const ImplicitTupleCopyExpr& value);
  bool traverse_unary_operator(const UnaryOperator& value);

  Value compile_id_expression(const IdExpr& value);
  Value compile_call_expression(const CallExpr& value);
  Value compile_integer_literal(const IntegerLiteral& value);
  Value compile_bool_literal(const BoolLiteral& value);
  Value compile_string_literal(const StringLiteral& value);
  Value compile_tuple_index_expression(const TupleIndexExpr& value);
  Value compile_tuple_expression(const TupleExpr& value);
  Value compile_member_expression(const MemberExpr& value);
  Value compile_binary_operator(const BinaryOperator& value);
  Value compile_unary_operator(const UnaryOperator& value);

  std::unique_ptr<llvm::Module> compile();
};
}  // namespace Front
