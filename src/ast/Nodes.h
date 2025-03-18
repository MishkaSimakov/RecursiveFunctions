#pragma once
#include <compilation/StringId.h>

#include <bitset>
#include <memory>
#include <ranges>
#include <vector>

#include "compilation/types/Type.h"
#include "errors/Helpers.h"
#include "lexis/Token.h"

// Here all AST Nodes are defined. To add new node:
// 1. Create class and inherit it from ASTNode
// 2. Add new ASTNode::Kind
// 3. Register node in NodesList.h
// 4. Add traverse_* method in ASTVisitor
// 5. Add visit_* method in ASTPrinter

namespace Front {
struct QualifiedId {
  std::vector<StringId> parts;
};

struct ASTNode {
  // clang-format off
  ENUM(Kind,
    COMPOUND_STMT,
    PROGRAM_DECL,
    FUNCTION_DECL,
    RETURN_STMT,
    INTEGER_LITERAL_EXPR,
    STRING_LITERAL_EXPR,
    BOOL_LITERAL_EXPR,
    ID_EXPR,
    IMPORT_DECL,
    BINARY_OPERATOR_EXPR,
    UNARY_OPERATOR_EXPR,
    CALL_EXPR,
    TYPE_NODE,
    VARIABLE_DECL,
    ASSIGNMENT_STMT,
    DECLARATION_STMT,
    EXPRESSION_STMT,
    NAMESPACE_DECL,
    WHILE_STMT,
    IF_STMT,
    CONTINUE_STMT,
    BREAK_STMT,

    IMPLICIT_LVALUE_TO_RVALUE_CONVERSION_EXPR,
  );
  // clang-format on

  SourceRange source_range;

  explicit ASTNode(SourceRange source_range) : source_range(source_range) {}

  SourceLocation source_begin() const { return source_range.begin; }
  SourceLocation source_end() const { return source_range.end; }

  virtual ~ASTNode() = default;

  virtual Kind get_kind() const = 0;
};

struct Statement : ASTNode {
  using ASTNode::ASTNode;
};
struct Declaration : ASTNode {
  using ASTNode::ASTNode;
};
struct TypeNode : ASTNode {
  Type* value;

  TypeNode(SourceRange source_range, Type* value)
      : ASTNode(source_range), value(value) {}
  Kind get_kind() const override { return Kind::TYPE_NODE; }
};

struct NamedDecl : Declaration {
  StringId name;

  NamedDecl(SourceRange source_range, StringId name)
      : Declaration(source_range), name(name) {}
};

enum class ValueCategory {
  LVALUE,
  RVALUE,

  UNKNOWN
};

struct Expression : ASTNode {
  Type* type{nullptr};
  ValueCategory value_category{ValueCategory::UNKNOWN};

  using ASTNode::ASTNode;
};

struct CompoundStmt : Statement {
  std::vector<std::unique_ptr<Statement>> statements;

  CompoundStmt(SourceRange source_range) : Statement(source_range) {}

  void add_item(std::unique_ptr<Statement> node) {
    statements.push_back(std::move(node));
  }

  Kind get_kind() const override { return Kind::COMPOUND_STMT; }
};

struct ReturnStmt : Statement {
  std::unique_ptr<Expression> value;

  ReturnStmt(SourceRange source_range, std::unique_ptr<Expression> value)
      : Statement(source_range), value(std::move(value)) {}

  Kind get_kind() const override { return Kind::RETURN_STMT; }
};

struct IntegerLiteral : Expression {
  int value;

  IntegerLiteral(SourceRange source_range, int value)
      : Expression(source_range), value(value) {}

  Kind get_kind() const override { return Kind::INTEGER_LITERAL_EXPR; }
};

struct BoolLiteral : Expression {
  bool value;

  BoolLiteral(SourceRange source_range, bool value)
      : Expression(source_range), value(value) {}

  Kind get_kind() const override { return Kind::BOOL_LITERAL_EXPR; }
};

struct StringLiteral : Expression {
  StringId id;
  StringLiteral(SourceRange source_range, StringId id)
      : Expression(source_range), id(id) {}

  Kind get_kind() const override { return Kind::STRING_LITERAL_EXPR; }
};

struct IdExpr : Expression {
  QualifiedId id;

  IdExpr(SourceRange source_range, QualifiedId id)
      : Expression(source_range), id(std::move(id)) {}
  Kind get_kind() const override { return Kind::ID_EXPR; }
};

struct ImportDecl : Declaration {
  StringId id;

  ImportDecl(SourceRange source_range, StringId id)
      : Declaration(source_range), id(id) {}

  Kind get_kind() const override { return Kind::IMPORT_DECL; }
};

struct BinaryOperator : Expression {
  // clang-format off
  ENUM(OpType,
    PLUS,
    MINUS,
    MULTIPLY,
    REMAINDER,
    LESS,
    GREATER,
    LESS_EQ,
    GREATER_EQ,
    EQUALEQUAL,
    NOTEQUAL,
  );
  // clang-format on

  OpType op_type;
  std::unique_ptr<Expression> left;
  std::unique_ptr<Expression> right;

  BinaryOperator(SourceRange source_range, OpType op_type,
                 std::unique_ptr<Expression> left,
                 std::unique_ptr<Expression> right)
      : Expression(source_range),
        op_type(op_type),
        left(std::move(left)),
        right(std::move(right)) {}

  bool is_arithmetic() const {
    return op_type
        .in<OpType::PLUS, OpType::MINUS, OpType::MULTIPLY, OpType::REMAINDER>();
  }

  bool is_inequality() const {
    return op_type.in<OpType::LESS, OpType::LESS_EQ, OpType::GREATER,
                      OpType::GREATER_EQ>();
  }

  bool is_equality() const {
    return op_type.in<OpType::EQUALEQUAL, OpType::NOTEQUAL>();
  }

  std::string_view get_string_representation() const {
    switch (op_type) {
      case OpType::PLUS:
        return "+";
      case OpType::MINUS:
        return "-";
      case OpType::MULTIPLY:
        return "*";
      case OpType::REMAINDER:
        return "%";
      case OpType::LESS:
        return "<";
      case OpType::GREATER:
        return ">";
      case OpType::LESS_EQ:
        return "<=";
      case OpType::GREATER_EQ:
        return ">=";
      case OpType::EQUALEQUAL:
        return "==";
      case OpType::NOTEQUAL:
        return "!=";
      default:
        unreachable("All operators are presented above.");
    }
  }

  Kind get_kind() const override { return Kind::BINARY_OPERATOR_EXPR; }
};

struct UnaryOperator : Expression {
  enum class OpType {
    PLUS,
    MINUS,
    PREINCREMENT,
    PREDECREMENT,
  };

  OpType op_type;
  std::unique_ptr<Expression> value;

  UnaryOperator(SourceRange source_range, OpType op_type,
                std::unique_ptr<Expression> value)
      : Expression(source_range), op_type(op_type), value(std::move(value)) {}

  std::string_view get_string_representation() const {
    switch (op_type) {
      case OpType::PLUS:
        return "+";
      case OpType::MINUS:
        return "-";
      case OpType::PREINCREMENT:
        return "++";
      case OpType::PREDECREMENT:
        return "--";
      default:
        unreachable("All operators are presented above.");
    }
  }

  Kind get_kind() const override { return Kind::UNARY_OPERATOR_EXPR; }
};

struct CallExpr : Expression {
  std::unique_ptr<Expression> callee;
  std::vector<std::unique_ptr<Expression>> arguments;

  CallExpr(SourceRange source_range, std::unique_ptr<Expression> callee,
           std::vector<std::unique_ptr<Expression>> arguments)
      : Expression(source_range),
        callee(std::move(callee)),
        arguments(std::move(arguments)) {}

  Kind get_kind() const override { return Kind::CALL_EXPR; }
};

struct VariableDecl : NamedDecl {
  std::unique_ptr<TypeNode> type;
  std::unique_ptr<Expression> initializer;

  VariableDecl(SourceRange source_range, StringId name,
               std::unique_ptr<TypeNode> type,
               std::unique_ptr<Expression> initializer)
      : NamedDecl(source_range, name),
        type(std::move(type)),
        initializer(std::move(initializer)) {}

  Kind get_kind() const override { return Kind::VARIABLE_DECL; }
};

struct AssignmentStmt : Statement {
  std::unique_ptr<Expression> left;
  std::unique_ptr<Expression> right;

  AssignmentStmt(SourceRange source_range, std::unique_ptr<Expression> left,
                 std::unique_ptr<Expression> right)
      : Statement(source_range),
        left(std::move(left)),
        right(std::move(right)) {}

  Kind get_kind() const override { return Kind::ASSIGNMENT_STMT; }
};

struct FunctionDecl : NamedDecl {
  struct Specifiers {
   private:
    constexpr static size_t kExportedId = 0;
    constexpr static size_t kExternId = 1;

    std::bitset<2> specifiers_{0};

   public:
    Specifiers() = default;

    Specifiers& set_exported(bool is_exported) {
      specifiers_[kExportedId] = is_exported;
      return *this;
    }

    Specifiers& set_extern(bool is_extern) {
      specifiers_[kExternId] = is_extern;
      return *this;
    }

    bool is_exported() const { return specifiers_[kExportedId]; }

    bool is_extern() const { return specifiers_[kExternId]; }
  };

  std::vector<std::unique_ptr<VariableDecl>> parameters;
  std::unique_ptr<TypeNode> return_type;
  std::unique_ptr<CompoundStmt> body;
  Specifiers specifiers;

  FunctionDecl(SourceRange source_range, StringId name,
               std::vector<std::unique_ptr<VariableDecl>> parameters,
               std::unique_ptr<TypeNode> return_type,
               std::unique_ptr<CompoundStmt> body, Specifiers specifiers)
      : NamedDecl(source_range, name),
        parameters(std::move(parameters)),
        return_type(std::move(return_type)),
        body(std::move(body)),
        specifiers(specifiers) {}

  Kind get_kind() const override { return Kind::FUNCTION_DECL; }
};

struct ProgramDecl : Declaration {
  std::vector<std::unique_ptr<ImportDecl>> imports;
  std::vector<std::unique_ptr<Declaration>> declarations;

  ProgramDecl(SourceRange source_range) : Declaration(source_range) {}

  Kind get_kind() const override { return Kind::PROGRAM_DECL; }
};

struct DeclarationStmt : Statement {
  std::unique_ptr<Declaration> value;

  DeclarationStmt(SourceRange source_range,
                  std::unique_ptr<Declaration> declaration)
      : Statement(source_range), value(std::move(declaration)) {}

  Kind get_kind() const override { return Kind::DECLARATION_STMT; }
};

struct ExpressionStmt : Statement {
  std::unique_ptr<Expression> value;

  ExpressionStmt(SourceRange source_range,
                 std::unique_ptr<Expression> expression)
      : Statement(source_range), value(std::move(expression)) {}

  Kind get_kind() const override { return Kind::EXPRESSION_STMT; }
};

struct NamespaceDecl : NamedDecl {
  std::vector<std::unique_ptr<Declaration>> body;

  bool is_exported;

  NamespaceDecl(SourceRange source_range, StringId name,
                std::vector<std::unique_ptr<Declaration>> body,
                bool is_exported)
      : NamedDecl(source_range, name),
        body(std::move(body)),
        is_exported(is_exported) {}

  Kind get_kind() const override { return Kind::NAMESPACE_DECL; }
};

struct WhileStmt : Statement {
  std::unique_ptr<Expression> condition;
  std::unique_ptr<CompoundStmt> body;

  WhileStmt(SourceRange source_range, std::unique_ptr<Expression> condition,
            std::unique_ptr<CompoundStmt> body)
      : Statement(source_range),
        condition(std::move(condition)),
        body(std::move(body)) {}

  Kind get_kind() const override { return Kind::WHILE_STMT; }
};

struct IfStmt : Statement {
  std::unique_ptr<Expression> condition;
  std::unique_ptr<CompoundStmt> true_branch;
  std::unique_ptr<CompoundStmt> false_branch;

  IfStmt(SourceRange source_range, std::unique_ptr<Expression> condition,
         std::unique_ptr<CompoundStmt> true_branch,
         std::unique_ptr<CompoundStmt> false_branch)
      : Statement(source_range),
        condition(std::move(condition)),
        true_branch(std::move(true_branch)),
        false_branch(std::move(false_branch)) {}

  Kind get_kind() const override { return Kind::IF_STMT; }
};

struct ContinueStmt : Statement {
  explicit ContinueStmt(SourceRange source_range) : Statement(source_range) {}
  Kind get_kind() const override { return Kind::CONTINUE_STMT; }
};

struct BreakStmt : Statement {
  explicit BreakStmt(SourceRange source_range) : Statement(source_range) {}
  Kind get_kind() const override { return Kind::BREAK_STMT; }
};

// implicit nodes (added by SemanticAnalyzer)
struct ImplicitLvalueToRvalueConversionExpr : Expression {
  std::unique_ptr<Expression> value;

  ImplicitLvalueToRvalueConversionExpr(SourceRange source_range,
                                       std::unique_ptr<Expression> value)
      : Expression(source_range), value(std::move(value)) {}

  Kind get_kind() const override {
    return Kind::IMPLICIT_LVALUE_TO_RVALUE_CONVERSION_EXPR;
  }
};

// ASTNodes that follow this line are supplementary.
// They are used only in Parser. They must not be presented in AST after parsing

struct SupplementaryNode : ASTNode {
  using ASTNode::ASTNode;

  Kind get_kind() const override {
    unreachable("Token node doesn't appear anywhere except ASTBuildContext.");
  }
};

struct TokenNode final : SupplementaryNode {
  Lexis::TokenType token_type;

  TokenNode(const Lexis::Token& token)
      : SupplementaryNode(token.source_range), token_type(token.type) {}
  TokenNode(SourceRange source_range, Lexis::TokenType token_type)
      : SupplementaryNode(source_range), token_type(token_type) {}
};

template <typename NodeT = ASTNode>
  requires std::is_base_of_v<ASTNode, NodeT>
struct NodesList final : SupplementaryNode {
  std::vector<std::unique_ptr<NodeT>> nodes;

  using SupplementaryNode::SupplementaryNode;

  void add_item(std::unique_ptr<NodeT> node) {
    nodes.push_back(std::move(node));
  }
};

struct FunctionSpecifiersNode : SupplementaryNode, FunctionDecl::Specifiers {
  using SupplementaryNode::SupplementaryNode;
};

}  // namespace Front
