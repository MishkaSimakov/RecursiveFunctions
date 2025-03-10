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

llvm::Value* Front::IRASTVisitor::compile_expression(const Expression& expr) {
  traverse(expr);
  assert("Expression value must be calculated.");
  return current_expr_value_;
}

bool Front::IRASTVisitor::traverse_variable_declaration(
    const VariableDecl& value) {
  llvm::IRBuilder<> temp_builder(llvm_context_);

  auto& entry_block =
      llvm_ir_builder_->GetInsertBlock()->getParent()->getEntryBlock();
  temp_builder.SetInsertPoint(&entry_block);

  auto var_alloca = temp_builder.CreateAlloca(map_type(value.type->value));
  local_variables_.emplace(&value, var_alloca);

  if (value.initializer != nullptr) {
    auto initial_value = compile_expression(*value.initializer);
    llvm_ir_builder_->CreateStore(initial_value, var_alloca);
  }

  return true;
}

bool Front::IRASTVisitor::traverse_id_expression(const IdExpr& value) {
  Declaration& id_declaration = module_.symbols_info.at(&value)->declaration;
  auto* id_alloca = local_variables_.at(&id_declaration);
  current_expr_value_ =
      llvm_ir_builder_->CreateLoad(id_alloca->getAllocatedType(), id_alloca);

  return true;
}

bool Front::IRASTVisitor::traverse_binary_operator(
    const BinaryOperator& value) {
  auto left_op = compile_expression(*value.left);
  auto right_op = compile_expression(*value.right);

  switch (value.op_type) {
    case BinaryOperator::OpType::PLUS:
      current_expr_value_ = llvm_ir_builder_->CreateAdd(left_op, right_op);
      break;
    default:
      throw std::runtime_error("Not implemented.");
  }

  return true;
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

  for (size_t i = 0; i < value.parameters.size(); ++i) {
    traverse(*value.parameters[i]);

    // load parameter into allocated local variable
    llvm::Value* param_value = func->getArg(i);
    llvm_ir_builder_->CreateStore(param_value,
                                  local_variables_[value.parameters[i].get()]);
  }

  traverse(*value.body);

  llvm::verifyFunction(*func, &llvm::outs());
  local_variables_.clear();

  return true;
}

bool Front::IRASTVisitor::traverse_return_statement(const ReturnStmt& value) {
  auto return_value = compile_expression(*value.value);
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
