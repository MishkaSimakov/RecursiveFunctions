#pragma once
#include "ast/ASTVisitor.h"
#include "compilation/GlobalContext.h"

// This class fills up symbols table inside Scope
// + calculates type for each expression in AST
class TypesASTVisitor
    : public ASTVisitor<TypesASTVisitor, false, Order::POSTORDER> {
  GlobalContext& context_;
  ModuleContext& module_;

  TypesStorage& types();

  [[noreturn]] void scold_user(SourceLocation location,
                               const std::string& message);

  Type* name_lookup(Scope* base_scope, StringId name) const;
  Type* recursive_global_name_lookup(const ModuleContext& module,
                                     StringId name) const;

  void check_call_arguments(FunctionType* type, const CallExpr& call);

 public:
  TypesASTVisitor(ASTNode& root, GlobalContext& context, ModuleContext& module)
      : ASTVisitor(root), context_(context), module_(module) {}

  bool visit_function_declaration(FunctionDecl& node);

  bool visit_variable_declaration(VariableDecl& node);
  bool visit_parameter_declaration(ParameterDecl& node);

  bool visit_integer_literal(IntegerLiteral& node);
  bool visit_string_literal(StringLiteral& node);

  bool visit_id_expression(IdExpr& node);
  bool visit_call_expression(CallExpr& node);
  bool visit_binary_operator(BinaryOperator& node);

  bool visit_return_statement(ReturnStmt& node);
};
