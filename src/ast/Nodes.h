#pragma once
#include <memory>
#include <string>
#include <vector>

#include "lexis/Token.h"

struct ASTNode {
  enum class Kind {
    TOKEN,
    INT_TYPE,
    COMPOUND_STMT,
    PROGRAM_DECL,
    PARAMETER_DECL,
    PARAMETERS_LIST_DECL,
    FUNCTION_DECL,
    RETURN_STMT,
    INTEGER_LITERAL_EXPR,
    STRING_LITERAL_EXPR,
    ID_EXPR,
    IMPORT_DECL
  };

  SourceLocation source_begin;
  SourceLocation source_end;

  ASTNode(SourceRange source_range)
      : source_begin(source_range.begin), source_end(source_range.end) {}

  SourceRange source_range() const {
    return {source_begin, source_end};
  }

  virtual ~ASTNode() = default;

  virtual Kind get_kind() const = 0;
};

struct TokenNode : ASTNode {
  Lexis::TokenType token_type;

  TokenNode(const Lexis::Token& token)
      : ASTNode(token.source_range), token_type(token.type) {}

  Kind get_kind() const override { return Kind::TOKEN; }
};

struct Statement : ASTNode {
  using ASTNode::ASTNode;
};
struct Declaration : ASTNode {
  using ASTNode::ASTNode;
};
struct Type : ASTNode {
  using ASTNode::ASTNode;
};
struct Expression : ASTNode {
  using ASTNode::ASTNode;
};

struct IntType : Type {
  IntType(SourceRange source_range) : Type(source_range) {}

  Kind get_kind() const override { return Kind::INT_TYPE; }
};

struct CompoundStmt : Statement {
  std::vector<std::unique_ptr<Statement>> statements;

  CompoundStmt(SourceRange source_range)
      : Statement(source_range) {}

  void add_item(std::unique_ptr<Statement> node) {
    statements.push_back(std::move(node));
  }

  Kind get_kind() const override { return Kind::COMPOUND_STMT; }
};

struct ProgramDecl : Declaration {
  std::vector<std::unique_ptr<Declaration>> declarations;

  ProgramDecl(SourceRange source_range)
      : Declaration(source_range) {}

  void add_item(std::unique_ptr<Declaration> node) {
    declarations.push_back(std::move(node));
  }

  Kind get_kind() const override { return Kind::PROGRAM_DECL; }
};

struct ParameterDecl : Declaration {
  std::string id;
  std::unique_ptr<Type> type;

  ParameterDecl(SourceRange source_range, std::string_view id,
                std::unique_ptr<Type> type)
      : Declaration(source_range), id(id), type(std::move(type)) {}

  Kind get_kind() const override { return Kind::PARAMETER_DECL; }
};

struct ParametersListDecl : Declaration {
  std::vector<std::unique_ptr<ParameterDecl>> parameters;

  ParametersListDecl(SourceRange source_range)
      : Declaration(source_range) {}

  void add_item(std::unique_ptr<ParameterDecl> node) {
    parameters.push_back(std::move(node));
  }

  Kind get_kind() const override { return Kind::PARAMETERS_LIST_DECL; }
};

struct FunctionDecl : Declaration {
  std::string name;
  std::vector<std::unique_ptr<ParameterDecl>> parameters;
  std::unique_ptr<Type> return_type;
  std::unique_ptr<CompoundStmt> body;

  FunctionDecl(SourceRange source_range, std::string_view name,
               std::vector<std::unique_ptr<ParameterDecl>> parameters,
               std::unique_ptr<Type> return_type,
               std::unique_ptr<CompoundStmt> body)
      : Declaration(source_range),
        name(name),
        parameters(std::move(parameters)),
        return_type(std::move(return_type)),
        body(std::move(body)) {}

  Kind get_kind() const override { return Kind::FUNCTION_DECL; }
};

struct ReturnStmt : Statement {
  std::unique_ptr<Expression> value;

  ReturnStmt(SourceRange source_range,
             std::unique_ptr<Expression> value)
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
  std::string name;

  IdExpr(SourceRange source_range, std::string_view value)
      : Expression(source_range), name(value) {}

  Kind get_kind() const override { return Kind::ID_EXPR; }
};

struct ImportDecl : Declaration {
  size_t id;

  ImportDecl(SourceRange source_range, size_t id)
      : Declaration(source_range), id(id) {}

  Kind get_kind() const override { return Kind::IMPORT_DECL; }
};
