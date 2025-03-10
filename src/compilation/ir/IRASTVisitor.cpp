#include "IRASTVisitor.h"

llvm::Type* Front::IRASTVisitor::map_type(Type* type) const {
  switch (type->get_kind()) {
    case Type::Kind::INT:
      return llvm::Type::getInt64Ty(llvm_context_);
    case Type::Kind::BOOL:
      return llvm::Type::getInt1Ty(llvm_context_);
    case Type::Kind::VOID:
      return llvm::Type::getVoidTy(llvm_context_);
    default:
      throw std::runtime_error("Not implemented");
  }
}

llvm::Value* Front::IRASTVisitor::compile_expression(Expression* expr) {
  traverse(*expr);
  return current_expr_value_;
}

bool Front::IRASTVisitor::traverse_function_declaration(
    const FunctionDecl& value) {
  std::vector<llvm::Type*> arguments;
  for (auto& parameter : value.parameters) {
    arguments.emplace_back(map_type(parameter->type->value));
  }

  auto llvm_func_type = llvm::FunctionType::get(
      map_type(value.return_type->value), arguments, false);

  auto name = module_.get_string(value.name);
  auto func =
      llvm::Function::Create(llvm_func_type, llvm::Function::ExternalLinkage,
                             name, llvm_module_.get());

  auto basic_block = llvm::BasicBlock::Create(llvm_context_, "entry", func);
  llvm_ir_builder_->SetInsertPoint(basic_block);

  traverse(*value.body);

  llvm::verifyFunction(*func, &llvm::outs());
  return true;
}

bool Front::IRASTVisitor::traverse_return_statement(const ReturnStmt& value) {
  auto return_value = compile_expression(value.value.get());
  llvm_ir_builder_->CreateRet(return_value);
  return true;
}

bool Front::IRASTVisitor::visit_integer_literal(const IntegerLiteral& value) {
  current_expr_value_ = llvm_ir_builder_->getInt64(value.value);
  return true;
}

std::unique_ptr<llvm::Module> Front::IRASTVisitor::compile() {
  traverse(*module_.ast_root);
  return std::move(llvm_module_);
}
