#pragma once
#include <memory>
#include <string>
#include <vector>

#include "errors/Helpers.h"
#include "lexis/Token.h"
#include "compilation/types/Type.h"

struct ASTNode {
  enum class Kind {
    INT_TYPE,
    BOOL_TYPE,
    POINTER_TYPE,
    COMPOUND_STMT,
    PROGRAM_DECL,
    PARAMETER_DECL,
    FUNCTION_DECL,
    RETURN_STMT,
    INTEGER_LITERAL_EXPR,
    STRING_LITERAL_EXPR,
    ID_EXPR,
    IMPORT_DECL,
    BINARY_OPERATOR,
    CALL_EXPR,
  };

  SourceLocation source_begin;
  SourceLocation source_end;

  ASTNode(SourceRange source_range)
      : source_begin(source_range.begin), source_end(source_range.end) {}

  SourceRange source_range() const { return {source_begin, source_end}; }

  virtual ~ASTNode() = default;

  virtual Kind get_kind() const = 0;
};

struct TokenNode : ASTNode {
  Lexis::TokenType token_type;

  TokenNode(const Lexis::Token& token)
      : ASTNode(token.source_range), token_type(token.type) {}
  TokenNode(SourceRange source_range, Lexis::TokenType token_type)
      : ASTNode(source_range), token_type(token_type) {}

  Kind get_kind() const override {
    unreachable("Token node doesn't appear anywhere except ASTBuildContext.");
  }
};

struct Statement : ASTNode {
  using ASTNode::ASTNode;
};
struct Declaration : ASTNode {
  using ASTNode::ASTNode;
};
struct TypeNode : ASTNode {
  using ASTNode::ASTNode;

  Type* value;

  virtual size_t hash() const = 0;
};

struct Expression : ASTNode {
  Type* type;

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

struct ParameterDecl : Declaration {
  size_t id;
  Type* type;

  ParameterDecl(SourceRange source_range, size_t id, Type* type)
      : Declaration(source_range), id(id), type(std::move(type)) {}

  Kind get_kind() const override { return Kind::PARAMETER_DECL; }
};

template <typename NodeT = ASTNode>
  requires std::is_base_of_v<ASTNode, NodeT>
struct NodesList : ASTNode {
  std::vector<std::unique_ptr<NodeT>> nodes;

  NodesList(SourceRange source_range) : ASTNode(source_range) {}

  void add_item(std::unique_ptr<NodeT> node) {
    nodes.push_back(std::move(node));
  }

  Kind get_kind() const override {
    unreachable(
        "NodesList<T> node doesn't appear anywhere except ASTBuildContext.");
  }
};

struct FunctionDecl : Declaration {
  size_t name_id;
  std::vector<std::unique_ptr<ParameterDecl>> parameters;
  Type* return_type;
  std::unique_ptr<CompoundStmt> body;

  bool is_exported;

  FunctionDecl(SourceRange source_range, size_t name_id,
               std::vector<std::unique_ptr<ParameterDecl>> parameters,
               Type* return_type, std::unique_ptr<CompoundStmt> body,
               bool is_exported)
      : Declaration(source_range),
        name_id(name_id),
        parameters(std::move(parameters)),
        return_type(std::move(return_type)),
        body(std::move(body)),
        is_exported(is_exported) {}

  Kind get_kind() const override { return Kind::FUNCTION_DECL; }
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

struct StringLiteral : Expression {
  size_t id;
  StringLiteral(SourceRange source_range, size_t id)
      : Expression(source_range), id(id) {}

  Kind get_kind() const override { return Kind::STRING_LITERAL_EXPR; }
};

struct IdExpr : Expression {
  size_t id;

  IdExpr(SourceRange source_range, size_t id)
      : Expression(source_range), id(id) {}

  Kind get_kind() const override { return Kind::ID_EXPR; }
};

struct ImportDecl : Declaration {
  size_t id;

  ImportDecl(SourceRange source_range, size_t id)
      : Declaration(source_range), id(id) {}

  Kind get_kind() const override { return Kind::IMPORT_DECL; }
};

struct BinaryOperator : Expression {
  enum class OpType { PLUS, MINUS, MULTIPLY };

  OpType type;
  std::unique_ptr<Expression> left;
  std::unique_ptr<Expression> right;

  BinaryOperator(SourceRange source_range, OpType type,
                 std::unique_ptr<Expression> left,
                 std::unique_ptr<Expression> right)
      : Expression(source_range),
        type(type),
        left(std::move(left)),
        right(std::move(right)) {}

  Kind get_kind() const override { return Kind::BINARY_OPERATOR; }
};

struct CallExpr : Expression {
  size_t id;
  std::vector<std::unique_ptr<Expression>> arguments;

  CallExpr(SourceRange source_range, size_t id,
           std::vector<std::unique_ptr<Expression>> arguments)
      : Expression(source_range), id(id), arguments(std::move(arguments)) {}

  Kind get_kind() const override { return Kind::CALL_EXPR; }
};

struct ProgramDecl : Declaration {
  std::vector<std::unique_ptr<ImportDecl>> imports;
  std::vector<std::unique_ptr<Declaration>> declarations;

  ProgramDecl(SourceRange source_range) : Declaration(source_range) {}

  Kind get_kind() const override { return Kind::PROGRAM_DECL; }
};
