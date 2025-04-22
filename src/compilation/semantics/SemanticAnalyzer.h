#pragma once
#include "ast/ASTVisitor.h"
#include "compilation/GlobalContext.h"
#include "utils/OneShotObject.h"

namespace Front {
template <typename T>
struct is_type_ptr : std::bool_constant<false> {};

template <typename T>
  requires std::is_base_of_v<Type, T>
struct is_type_ptr<T*> : std::bool_constant<true> {};

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
  ModuleContext& context_;
  std::vector<std::pair<SourceRange, std::string>> errors_;

  Scope* current_scope_{nullptr};
  size_t anonymous_namespace_counter_{0};

  TypesStorage& types();
  SymbolInfo* name_lookup(Scope* scope, const QualifiedId& id);

  void add_to_exported_if_necessary(SymbolInfo& info);

  Scope* inject_nested_scope(const ModuleContext& source_module,
                             QualifiedId source_namespaces);
  void inject_symbol(ModuleContext& source_module, SymbolInfo& source_symbol);
  void inject_symbol_to(Scope* scope, ModuleContext& source_module,
                        SymbolInfo& source_symbol);
  Type* inject_type(Type* external_type, const StringPool& external_strings);

  [[noreturn]] void scold_user(const ASTNode& node, std::string message);

  template <typename... Args>
  [[noreturn]] void scold_user(const ASTNode& node, std::string_view format,
                               Args&&... args) {
    // special formatter for types
    auto format_type = [this]<typename T>(T&& value) {
      if constexpr (is_type_ptr<std::decay_t<T>>::value) {
        return value->to_string(context_.get_strings_pool());
      } else if constexpr (std::is_same_v<std::decay_t<T>, StringId>) {
        return context_.get_string(value);
      } else {
        return std::forward<T>(value);
      }
    };

    scold_user(node, fmt::format(fmt::runtime(format), format_type(args)...));
  }

  Type* add_to_transformations_if_necessary(const FunctionSymbolInfo& function);
  bool is_transformation(CallExpr& node);

  class NestedScopeRAII {
    Scope*& current_scope_;

   public:
    // starts new anonymous namespace
    explicit NestedScopeRAII(SemanticAnalyzer& analyzer, Scope& scope)
        : current_scope_(analyzer.current_scope_) {
      current_scope_ = &scope;
    }

    NestedScopeRAII(const NestedScopeRAII&) = delete;
    NestedScopeRAII& operator=(const NestedScopeRAII&) = delete;

    NestedScopeRAII(NestedScopeRAII&&) = delete;
    NestedScopeRAII& operator=(NestedScopeRAII&&) = delete;

    ~NestedScopeRAII() { current_scope_ = current_scope_->parent; }
  };

  void convert_to_rvalue(std::unique_ptr<Expression>& expression);

  // this function is called for function arguments and initializer expressions
  // for variables.
  void as_initializer(std::unique_ptr<Expression>& expression);

 public:
  bool after_traverse(ASTNode& node);

  explicit SemanticAnalyzer(ModuleContext& context) : context_(context) {}

  // statements
  bool traverse_compound_statement(CompoundStmt& node);
  bool visit_return_statement(ReturnStmt& node);
  bool visit_assignment_statement(AssignmentStmt& node);
  bool traverse_if_statement(IfStmt& node);

  // declarations
  bool traverse_function_declaration(FunctionDecl& node);
  bool visit_variable_declaration(VariableDecl& node);
  bool traverse_namespace_declaration(NamespaceDecl& node);
  bool visit_type_alias_declaration(TypeAliasDecl& node);
  bool traverse_class_declaration(ClassDecl& node);

  // expressions
  bool visit_integer_literal(IntegerLiteral& node);
  bool visit_string_literal(StringLiteral& node);
  bool visit_bool_literal(BoolLiteral& node);
  bool visit_id_expression(IdExpr& node);
  bool visit_binary_operator(BinaryOperator& node);
  bool visit_unary_operator(UnaryOperator& node);
  bool visit_call_expression(CallExpr& node);
  bool visit_member_expression(MemberExpr& node);
  bool visit_tuple_expression(TupleExpr& node);
  bool visit_tuple_index_expression(TupleIndexExpr& node);
  bool visit_explicit_unsafe_cast_expression(ExplicitUnsafeCastExpr& node);

  // types
  bool visit_pointer_type(PointerTypeNode& node);
  bool visit_primitive_type(PrimitiveTypeNode& node);
  bool visit_tuple_type(TupleTypeNode& node);
  bool visit_user_defined_type(UserDefinedTypeNode& node);

  void analyze();

  friend struct SymbolInjector;
};
}  // namespace Front
