#pragma once
#include <compilation/StringId.h>

#include <memory>
#include <ranges>
#include <string>
#include <vector>

#include "compilation/Scope.h"
#include "compilation/types/Type.h"
#include "errors/Helpers.h"
#include "lexis/Token.h"

// Here all AST Nodes are defined. To add new node:
// 1. Create class and inherit it from ASTNode
// 2. Add new ASTNode::Kind
// 3. Register node in NodesList.h
// 4. Add traverse_* method in ASTVisitor
// 5. Add visit_* method in ASTPrinter

struct ASTNode {
  enum class Kind {
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
    TYPE_NODE,
    VARIABLE_DECL,
    DECLARATION_STMT,
  };

  SourceRange source_range;
  Scope* scope;

  ASTNode(SourceRange source_range)
      : source_range(source_range), scope(nullptr) {}

  SourceLocation source_begin() const { return source_range.begin; }
  SourceLocation source_end() const { return source_range.end; }

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
  Type* value;

  TypeNode(SourceRange source_range, Type* value)
      : ASTNode(source_range), value(value) {}
  Kind get_kind() const override { return Kind::TYPE_NODE; }
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
  StringId id;
  std::unique_ptr<TypeNode> type;

  ParameterDecl(SourceRange source_range, StringId id,
                std::unique_ptr<TypeNode> type)
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
  StringId name;
  std::vector<std::unique_ptr<ParameterDecl>> parameters;
  std::unique_ptr<TypeNode> return_type;
  std::unique_ptr<CompoundStmt> body;

  bool is_exported;

  FunctionDecl(SourceRange source_range, StringId name,
               std::vector<std::unique_ptr<ParameterDecl>> parameters,
               std::unique_ptr<TypeNode> return_type,
               std::unique_ptr<CompoundStmt> body, bool is_exported)
      : Declaration(source_range),
        name(name),
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
  StringId id;
  StringLiteral(SourceRange source_range, StringId id)
      : Expression(source_range), id(id) {}

  Kind get_kind() const override { return Kind::STRING_LITERAL_EXPR; }
};

struct IdExpr : Expression {
  StringId id;

  IdExpr(SourceRange source_range, StringId id)
      : Expression(source_range), id(id) {}

  Kind get_kind() const override { return Kind::ID_EXPR; }
};

struct ImportDecl : Declaration {
  StringId id;

  ImportDecl(SourceRange source_range, StringId id)
      : Declaration(source_range), id(id) {}

  Kind get_kind() const override { return Kind::IMPORT_DECL; }
};

struct BinaryOperator : Expression {
  enum class OpType { PLUS, MINUS, MULTIPLY };

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

  Kind get_kind() const override { return Kind::BINARY_OPERATOR; }
};

struct CallExpr : Expression {
  StringId id;
  std::vector<std::unique_ptr<Expression>> arguments;

  CallExpr(SourceRange source_range, StringId id,
           std::vector<std::unique_ptr<Expression>> arguments)
      : Expression(source_range), id(id), arguments(std::move(arguments)) {}

  Kind get_kind() const override { return Kind::CALL_EXPR; }
};

struct VariableDecl : Declaration {
  StringId name;
  std::unique_ptr<TypeNode> type;
  std::unique_ptr<Expression> initializer;

  VariableDecl(SourceRange source_range, StringId name,
               std::unique_ptr<TypeNode> type,
               std::unique_ptr<Expression> initializer)
      : Declaration(source_range),
        name(name),
        type(std::move(type)),
        initializer(std::move(initializer)) {}

  Kind get_kind() const override { return Kind::VARIABLE_DECL; }
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
