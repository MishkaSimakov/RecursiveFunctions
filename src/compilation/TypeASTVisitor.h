#pragma once
#include <unordered_map>

#include "ast/ASTContext.h"
#include "ast/ASTVisitor.h"
#include "compilation/ModuleCompileInfo.h"
#include "sources/SourceManager.h"

class TypeASTVisitor {
  SourceManager& source_manager_;
  ModuleCompileInfo& current_;
  const std::unordered_map<std::string_view, ModuleCompileInfo>& modules_;
  Scope* current_scope_;

  std::pair<Scope*, > local_symbol_lookup(size_t index) const;
  Scope* global_symbol_lookup(size_t index) const; // TODO: implement

 public:
  TypeASTVisitor(
      SourceManager& source_manager, ModuleCompileInfo& current,
      const std::unordered_map<std::string_view, ModuleCompileInfo>& modules)
      : source_manager_(source_manager),
        current_(current),
        modules_(modules),
        current_scope_(&current.context.root_scope) {}

  bool after_traverse(ASTNode& node) {
    if (node.get_kind() == ASTNode::Kind::COMPOUND_STMT) {
      current_scope_ = current_scope_->parent;
    }
    return true;
  }

  bool visit_int_type(IntType& value) { return true; }
  bool visit_bool_type(BoolType& value) { return true; }
  bool visit_compound_statement(CompoundStmt& value) {
    auto& inner_scope = current_scope_->children.emplace_back();
    inner_scope.statement = &value;
    return true;
  }
  bool visit_program_declaration(const ProgramDecl& value) { return true; }
  bool visit_parameter_declaration(const ParameterDecl& value) { return true; }
  bool traverse_function_declaration(FunctionDecl& value);
  bool visit_return_statement(const ReturnStmt& value) { return true; }
  bool visit_integer_literal(const IntegerLiteral& value) { return true; }
  bool visit_string_literal(const StringLiteral& value) { return true; }
  bool visit_id_expression(const IdExpr& value);
  bool visit_import_declaration(const ImportDecl& value) { return true; }
  bool visit_call_expression(const CallExpr& value) { return true; }
  bool visit_binary_operator(const BinaryOperator& value) { return true; }
};
