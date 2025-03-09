#pragma once
#include "ast/ASTVisitor.h"
#include "compilation/GlobalContext.h"

namespace Front {
struct SemanticAnalyzerException : std::runtime_error {
  const std::vector<std::pair<SourceRange, std::string>> errors;

  explicit SemanticAnalyzerException(
      std::vector<std::pair<SourceRange, std::string>> errors)
      : std::runtime_error("Semantic error."), errors(std::move(errors)) {}
};

// This class performs semantic analysis
// Threew main tasks are solved here:
// 1. Calculate scope for each AST Node
// 2. Build symbol table
// 3. Construct types graph
class SemanticAnalyzer
    : public ASTVisitor<SemanticAnalyzer, false, Order::POSTORDER> {
  const GlobalContext& global_context_;
  ModuleContext& context_;
  std::vector<std::pair<SourceRange, std::string>> errors_;

  Scope* current_scope_{nullptr};

  TypesStorage& types();

  [[noreturn]] void scold_user(const ASTNode& node, std::string message);

  std::pair<Scope*, SymbolInfo*> name_lookup(Scope* base_scope, StringId id,
                                             bool should_ascend = true) const;

  std::pair<Scope*, SymbolInfo*> qualified_name_lookup(
      Scope* base_scope, const IdExpr& qualified_id);

  std::pair<Scope*, SymbolInfo*> recursive_global_name_lookup(
      const ModuleContext& module, StringId id) const;

  Scope* start_nested_scope() {
    auto& scope = current_scope_->children.emplace_back(
        std::make_unique<Scope>(current_scope_));

    current_scope_ = scope.get();
    return current_scope_;
  }

  void end_nested_scope() { current_scope_ = current_scope_->parent; }

  void check_call_arguments(FunctionType* type, const CallExpr& call);

 public:
  bool after_traverse(ASTNode& node);

  SemanticAnalyzer(const GlobalContext& global_context, ModuleContext& context)
      : global_context_(global_context), context_(context) {}

  bool before_program_declaration(ProgramDecl& node);
  bool after_program_declaration(ProgramDecl& node);

  bool before_compound_statement(CompoundStmt& node);
  bool after_compound_statement(CompoundStmt& node);

  bool visit_parameter_declaration(ParameterDecl& node);

  bool traverse_function_declaration(FunctionDecl& node);

  // bool before_return_statement(ReturnStmt& node);
  // bool after_return_statement(ReturnStmt& node);

  bool before_integer_literal(IntegerLiteral& node);
  bool before_string_literal(StringLiteral& node);
  bool before_bool_literal(BoolLiteral& node);

  bool visit_id_expression(IdExpr& node);

  // bool before_import_declaration(ImportDecl& node);
  // bool after_import_declaration(ImportDecl& node);

  bool visit_binary_operator(BinaryOperator& node);
  bool visit_unary_operator(UnaryOperator& node);

  bool visit_call_expression(CallExpr& node);

  // bool before_type_node(TypeNode& node);
  // bool after_type_node(TypeNode& node);

  bool visit_variable_declaration(VariableDecl& node);

  // bool before_declaration_statement(DeclarationStmt& node);
  // bool after_declaration_statement(DeclarationStmt& node);

  // bool before_expression_statement(ExpressionStmt& node);
  // bool after_expression_statement(ExpressionStmt& node);

  bool before_namespace_declaration(NamespaceDecl& node);
  bool after_namespace_declaration(NamespaceDecl& node);

  // bool before_while_statement(WhileStmt& node);
  // bool after_while_statement(WhileStmt& node);

  bool traverse_if_statement(IfStmt& node);

  // bool before_break_statement(BreakStmt& node);
  // bool after_break_statement(BreakStmt& node);

  // bool before_continue_statement(ContinueStmt& node);
  // bool after_continue_statement(ContinueStmt& node);

  void analyze() { traverse(*context_.ast_root); }
};
}  // namespace Front
