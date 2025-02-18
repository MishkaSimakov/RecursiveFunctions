#pragma once
#include "ast/ASTVisitor.h"
#include "compilation/ModuleContext.h"
#include "intermediate_representation/Program.h"

namespace Front {
class IRASTVisitor : public ASTVisitor<IRASTVisitor, true, Order::POSTORDER> {
  GlobalContext& context_;
  ModuleContext& module_;

  IR::Program program_;
  IR::Function* current_function_;
  IR::BasicBlock* current_basic_block_;
  IR::Value* result_location_;

  std::unordered_map<std::pair<StringId, Scope*>, IR::Temporary*>
      function_symbols_mapping_;

  IR::Type* map_type(Type* type);

  // compiler must not pass through some kind of nodes. If it does, then I've
  // made some mistake developing it
  [[noreturn]] static void unreachable_node() {
    unreachable("Compiler doesn't pass through this kind of nodes.");
  }

 public:
  IRASTVisitor(const ASTNode& root, GlobalContext& context,
               ModuleContext& module)
      : ASTVisitor(root), context_(context), module_(module) {}

  bool traverse_compound_statement(const CompoundStmt& value);
  bool traverse_program_declaration(const ProgramDecl& value);
  bool traverse_parameter_declaration(const ParameterDecl& value) {
    unreachable_node();
  }
  bool traverse_function_declaration(const FunctionDecl& value);
  bool traverse_return_statement(const ReturnStmt& value);
  bool traverse_integer_literal(const IntegerLiteral& value);
  bool traverse_string_literal(const StringLiteral& value);
  bool traverse_id_expression(const IdExpr& value);
  bool traverse_import_declaration(const ImportDecl& value) {
    unreachable_node();
  }
  bool traverse_binary_operator(const BinaryOperator& value);
  bool traverse_call_expression(const CallExpr& value);
  bool traverse_type_node(const TypeNode& value) { unreachable_node(); }
  bool traverse_variable_declaration(const VariableDecl& value);
  bool traverse_declaration_statement(const DeclarationStmt& value);
};
}  // namespace Front
