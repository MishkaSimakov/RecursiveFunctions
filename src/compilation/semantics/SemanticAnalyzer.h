#pragma once
#include "ast/ASTVisitor.h"
#include "compilation/GlobalContext.h"
#include "utils/OneShotObject.h"

namespace Front {
struct SemanticAnalyzerException : std::runtime_error {
  const std::vector<std::pair<SourceRange, std::string>> errors;

  explicit SemanticAnalyzerException(
      std::vector<std::pair<SourceRange, std::string>> errors)
      : std::runtime_error("Semantic error."), errors(std::move(errors)) {}
};

struct SemanticAnalyzerConfig : ASTVisitorConfig {
  static constexpr auto order() { return Order::POSTORDER; }
  static constexpr auto is_const() { return false; }
  static constexpr auto override_all() { return false; }
};

class SemanticAnalyzer
    : public ASTVisitor<SemanticAnalyzer, SemanticAnalyzerConfig>,
      OneShotObject {
  const GlobalContext& global_context_;
  ModuleContext& context_;
  std::vector<std::pair<SourceRange, std::string>> errors_;

  bool is_in_exported_scope_{false};
  Scope* current_scope_{nullptr};

  TypesStorage& types();
  SymbolInfo* name_lookup(Scope* scope, const QualifiedId& id);

  void inject_symbol(ModuleContext& module, SymbolInfo& symbol);
  Type* inject_type(Type* external_type);

  [[noreturn]] void scold_user(const ASTNode& node, std::string message);

  StringId import_external_string(StringId external_string,
                                  ModuleContext& external_module);
  QualifiedId import_external_string(QualifiedId external_string,
                                     ModuleContext& external_module);

  class NestedScopeRAII {
    Scope*& current_scope_;

   public:
    explicit NestedScopeRAII(Scope*& current_scope)
        : current_scope_(current_scope) {
      current_scope_ = &current_scope->add_child();
    }

    NestedScopeRAII(const NestedScopeRAII&) = delete;
    NestedScopeRAII& operator=(const NestedScopeRAII&) = delete;

    NestedScopeRAII(NestedScopeRAII&&) = delete;
    NestedScopeRAII& operator=(NestedScopeRAII&&) = delete;

    ~NestedScopeRAII() { current_scope_ = current_scope_->parent; }
  };

  void convert_to_rvalue(std::unique_ptr<Expression>& expression);

 public:
  bool after_traverse(ASTNode& node);

  SemanticAnalyzer(const GlobalContext& global_context, ModuleContext& context)
      : global_context_(global_context), context_(context) {}

  bool traverse_compound_statement(CompoundStmt& node);

  bool traverse_function_declaration(FunctionDecl& node);

  bool visit_return_statement(ReturnStmt& node);

  bool visit_integer_literal(IntegerLiteral& node);
  bool visit_string_literal(StringLiteral& node);
  bool visit_bool_literal(BoolLiteral& node);

  bool visit_id_expression(IdExpr& node);

  // bool before_import_declaration(ImportDecl& node);
  // bool after_import_declaration(ImportDecl& node);

  bool visit_binary_operator(BinaryOperator& node);
  bool visit_unary_operator(UnaryOperator& node);
  bool visit_call_expression(CallExpr& node);
  bool visit_assignment_statement(AssignmentStmt& node);

  // bool before_type_node(TypeNode& node);
  // bool after_type_node(TypeNode& node);

  bool visit_variable_declaration(VariableDecl& node);

  // bool before_declaration_statement(DeclarationStmt& node);
  // bool after_declaration_statement(DeclarationStmt& node);

  // bool before_expression_statement(ExpressionStmt& node);
  // bool after_expression_statement(ExpressionStmt& node);

  bool traverse_namespace_declaration(NamespaceDecl& node);

  // bool before_while_statement(WhileStmt& node);
  // bool after_while_statement(WhileStmt& node);

  bool traverse_if_statement(IfStmt& node);

  // bool before_break_statement(BreakStmt& node);
  // bool after_break_statement(BreakStmt& node);

  // bool before_continue_statement(ContinueStmt& node);
  // bool after_continue_statement(ContinueStmt& node);

  void analyze();
};
}  // namespace Front
