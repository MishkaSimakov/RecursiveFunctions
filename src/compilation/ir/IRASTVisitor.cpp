#include "IRASTVisitor.h"

#include <iostream>

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
  current_expr_value_ = nullptr;
  traverse(expr);
  assert(current_expr_value_ != nullptr &&
         "Expression value must be calculated.");
  return std::exchange(current_expr_value_, nullptr);
}

bool Front::IRASTVisitor::
    traverse_implicit_lvalue_to_rvalue_conversion_expression(
        const ImplicitLvalueToRvalueConversionExpr& value) {
  llvm::Value* ptr = compile_expression(*value.value);
  current_expr_value_ = llvm_ir_builder_->CreateLoad(map_type(value.type), ptr);
  return true;
}

bool Front::IRASTVisitor::traverse_assignment_statement(
    const AssignmentStmt& value) {
  auto left = compile_expression(*value.left);
  auto right = compile_expression(*value.right);

  llvm_ir_builder_->CreateStore(right, left);
  return true;
}

bool Front::IRASTVisitor::traverse_if_statement(const IfStmt& value) {
  llvm::Value* condition = compile_expression(*value.condition);
  llvm::Function* current_function =
      llvm_ir_builder_->GetInsertBlock()->getParent();

  auto* true_branch =
      llvm::BasicBlock::Create(llvm_context_, "true_br", current_function);
  auto* false_branch = llvm::BasicBlock::Create(llvm_context_, "false_br");
  auto* merge = llvm::BasicBlock::Create(llvm_context_, "ifcont");

  llvm_ir_builder_->CreateCondBr(condition, true_branch, false_branch);

  llvm_ir_builder_->SetInsertPoint(true_branch);
  traverse(*value.true_branch);
  llvm_ir_builder_->CreateBr(merge);

  current_function->insert(current_function->end(), false_branch);
  llvm_ir_builder_->SetInsertPoint(false_branch);
  traverse(*value.false_branch);
  llvm_ir_builder_->CreateBr(merge);

  current_function->insert(current_function->end(), merge);
  llvm_ir_builder_->SetInsertPoint(merge);
  return true;
}

bool Front::IRASTVisitor::traverse_call_expression(const CallExpr& value) {
  llvm::Function* callee =
      llvm::dyn_cast<llvm::Function>(compile_expression(*value.callee));

  std::vector<llvm::Value*> arguments;
  arguments.reserve(value.arguments.size());
  for (auto& argument : value.arguments) {
    arguments.emplace_back(compile_expression(*argument));
  }

  current_expr_value_ = llvm_ir_builder_->CreateCall(callee, arguments);
  return true;
}

bool Front::IRASTVisitor::traverse_variable_declaration(
    const VariableDecl& value) {
  llvm::IRBuilder<> temp_builder(llvm_context_);

  auto& entry_block =
      llvm_ir_builder_->GetInsertBlock()->getParent()->getEntryBlock();
  temp_builder.SetInsertPoint(&entry_block);

  auto var_alloca = temp_builder.CreateAlloca(map_type(value.type->value));
  identifiers_addresses_.emplace(&value, var_alloca);

  if (value.initializer != nullptr) {
    auto initial_value = compile_expression(*value.initializer);
    llvm_ir_builder_->CreateStore(initial_value, var_alloca);
  }

  return true;
}

bool Front::IRASTVisitor::traverse_id_expression(const IdExpr& value) {
  const SymbolInfo& info = module_.symbols_info.at(&value);

  if (value.type->get_kind() != Type::Kind::FUNCTION) {
    current_expr_value_ = identifiers_addresses_.at(&info.declaration);
  } else {
    auto name = mangler_.mangle(info);
    current_expr_value_ = llvm_module_->getFunction(name);
  }

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
    case BinaryOperator::OpType::MULTIPLY:
      current_expr_value_ = llvm_ir_builder_->CreateMul(left_op, right_op);
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

  const SymbolInfo& function_info = module_.functions_info.at(&value);
  std::string name = mangler_.mangle(function_info);
  auto func =
      llvm::Function::Create(llvm_func_type, llvm::Function::ExternalLinkage,
                             name, llvm_module_.get());

  // external function doesn't have a body
  if (value.specifiers.is_extern()) {
    return true;
  }

  auto basic_block = llvm::BasicBlock::Create(llvm_context_, "entry", func);
  llvm_ir_builder_->SetInsertPoint(basic_block);

  for (size_t i = 0; i < value.parameters.size(); ++i) {
    traverse(*value.parameters[i]);

    // load parameter into allocated local variable
    llvm::Value* param_value = func->getArg(i);
    llvm_ir_builder_->CreateStore(
        param_value, identifiers_addresses_[value.parameters[i].get()]);
  }

  traverse(*value.body);

  llvm::verifyFunction(*func, &llvm::outs());
  identifiers_addresses_.clear();

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

bool Front::IRASTVisitor::visit_bool_literal(const BoolLiteral& value) {
  current_expr_value_ = llvm_ir_builder_->getInt1(value.value);
  return true;
}

std::unique_ptr<llvm::Module> Front::IRASTVisitor::compile() {
  traverse(*module_.ast_root);
  return std::move(llvm_module_);
}
