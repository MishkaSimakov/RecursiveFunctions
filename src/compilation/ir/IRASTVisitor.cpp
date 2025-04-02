#include "IRASTVisitor.h"

#include <iostream>

namespace Front {
llvm::Type* IRASTVisitor::map_type(Type* type) const {
  switch (type->get_kind()) {
    case Type::Kind::SIGNED_INT:
    case Type::Kind::UNSIGNED_INT:
    case Type::Kind::CHAR: {
      auto* ptype = static_cast<PrimitiveType*>(type);
      return llvm::Type::getIntNTy(llvm_context_, ptype->width);
    }
    case Type::Kind::BOOL:
      return llvm::Type::getInt1Ty(llvm_context_);
    case Type::Kind::ALIAS: {
      AliasType* alias_ty = static_cast<AliasType*>(type);
      return map_type(alias_ty->original);
    }
    case Type::Kind::TUPLE: {
      TupleType* tuple_ty = static_cast<TupleType*>(type);

      std::vector<llvm::Type*> mapped_elements;
      for (Type* element : tuple_ty->elements) {
        mapped_elements.push_back(map_type(element));
      }
      return llvm::StructType::get(llvm_context_, std::move(mapped_elements));
    }
    case Type::Kind::CLASS: {
      ClassType* class_ty = static_cast<ClassType*>(type);

      auto name = class_ty->name.to_string(module_.get_strings_pool());
      std::vector<llvm::Type*> mapped_elements;
      for (Type* member : class_ty->members | std::views::values) {
        mapped_elements.push_back(map_type(member));
      }

      return llvm::StructType::create(llvm_context_, std::move(mapped_elements),
                                      name);
    }
    case Type::Kind::POINTER:
      return llvm::PointerType::get(llvm_context_, 0);
    default:
      not_implemented();
  }
}

llvm::Value* IRASTVisitor::compile_expression(const Expression& expr) {
  current_expr_value_ = nullptr;
  traverse(expr);
  assert(current_expr_value_ != nullptr &&
         "Expression value must be calculated.");
  return std::exchange(current_expr_value_, nullptr);
}

llvm::Function* IRASTVisitor::get_or_insert_function(
    const FunctionSymbolInfo& info) {
  std::string name = mangler_.mangle(info);
  llvm::Function* function = llvm_module_->getFunction(name);
  if (function != nullptr) {
    return function;
  }

  // create new function
  std::vector<llvm::Type*> arguments;
  for (Type* argument_type : info.type->arguments) {
    if (!argument_type->is_passed_by_value()) {
      argument_type = module_.types_storage.add_pointer(argument_type);
    }

    arguments.emplace_back(map_type(argument_type));
  }

  Type* ret_ty = info.type->return_type;
  llvm::Type* llvm_ret_ty = ret_ty->is_unit()
                                ? llvm::Type::getVoidTy(llvm_context_)
                                : map_type(ret_ty);

  auto llvm_func_type = llvm::FunctionType::get(llvm_ret_ty, arguments, false);

  return llvm::Function::Create(llvm_func_type, llvm::Function::ExternalLinkage,
                                name, llvm_module_.get());
}

void IRASTVisitor::allocate_local_variables(const FunctionSymbolInfo& info) {
  for (VariableSymbolInfo& variable : info.local_variables) {
    auto& declaration = static_cast<VariableDecl&>(variable.declaration);

    auto var_alloca = llvm_ir_builder_->CreateAlloca(map_type(variable.type));
    identifiers_addresses_.emplace(&declaration, var_alloca);
  }
}

bool IRASTVisitor::traverse_implicit_lvalue_to_rvalue_conversion_expression(
    const ImplicitLvalueToRvalueConversionExpr& value) {
  llvm::Value* ptr = compile_expression(*value.value);
  current_expr_value_ = llvm_ir_builder_->CreateLoad(map_type(value.type), ptr);
  return true;
}

bool IRASTVisitor::traverse_assignment_statement(const AssignmentStmt& value) {
  auto left = compile_expression(*value.left);
  auto right = compile_expression(*value.right);

  llvm_ir_builder_->CreateStore(right, left);
  return true;
}

bool IRASTVisitor::traverse_if_statement(const IfStmt& value) {
  llvm::Value* condition = compile_expression(*value.condition);
  llvm::Function* current_function =
      llvm_ir_builder_->GetInsertBlock()->getParent();

  auto* true_branch =
      llvm::BasicBlock::Create(llvm_context_, "true_br", current_function);
  auto* false_branch = llvm::BasicBlock::Create(llvm_context_, "false_br");
  auto* merge = llvm::BasicBlock::Create(llvm_context_, "ifcont");

  llvm_ir_builder_->CreateCondBr(condition, true_branch, false_branch);

  llvm_ir_builder_->SetInsertPoint(true_branch);
  if (traverse(*value.true_branch)) {
    llvm_ir_builder_->CreateBr(merge);
  }

  current_function->insert(current_function->end(), false_branch);
  llvm_ir_builder_->SetInsertPoint(false_branch);
  if (traverse(*value.false_branch)) {
    llvm_ir_builder_->CreateBr(merge);
  }

  current_function->insert(current_function->end(), merge);
  llvm_ir_builder_->SetInsertPoint(merge);
  return true;
}

bool IRASTVisitor::traverse_while_statement(const WhileStmt& value) {
  llvm::Function* function = llvm_ir_builder_->GetInsertBlock()->getParent();

  // create basic blocks
  auto* condition_block =
      llvm::BasicBlock::Create(llvm_context_, "condition", function);
  auto* loop_block = llvm::BasicBlock::Create(llvm_context_, "loop", function);
  auto* after_loop_block =
      llvm::BasicBlock::Create(llvm_context_, "after_loop", function);

  llvm_ir_builder_->CreateBr(condition_block);

  // compile loop condition
  llvm_ir_builder_->SetInsertPoint(condition_block);
  llvm::Value* condition = compile_expression(*value.condition);
  llvm_ir_builder_->CreateCondBr(condition, loop_block, after_loop_block);

  // compile loop body
  llvm_ir_builder_->SetInsertPoint(loop_block);
  if (traverse(*value.body)) {
    llvm_ir_builder_->CreateBr(condition_block);
  }

  //
  llvm_ir_builder_->SetInsertPoint(after_loop_block);

  return true;
}

bool IRASTVisitor::traverse_call_expression(const CallExpr& value) {
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

bool IRASTVisitor::traverse_variable_declaration(const VariableDecl& value) {
  if (value.initializer != nullptr) {
    llvm::Value* var_ptr = identifiers_addresses_.at(&value);
    auto initial_value = compile_expression(*value.initializer);
    llvm_ir_builder_->CreateStore(initial_value, var_ptr);
  }

  return true;
}

bool IRASTVisitor::traverse_id_expression(const IdExpr& value) {
  const SymbolInfo& info = module_.symbols_info.at(&value);

  std::visit(
      Overloaded{
          [&](const auto&) {
            unreachable(
                "SemanticAnalyzer ensures that other options are impossible.");
          },
          [&](const VariableSymbolInfo& var) {
            current_expr_value_ = identifiers_addresses_.at(&var.declaration);
          },
          [&](const FunctionSymbolInfo& fun) {
            current_expr_value_ = get_or_insert_function(fun);
          }},
      info);

  return true;
}

bool IRASTVisitor::traverse_binary_operator(const BinaryOperator& value) {
  auto left_op = compile_expression(*value.left);
  auto right_op = compile_expression(*value.right);

  auto get_llvm_binary_op_type = [](BinaryOperator::OpType op_type) {
    using BinaryOps = llvm::Instruction::BinaryOps;
    switch (op_type) {
      case BinaryOperator::OpType::PLUS:
        return BinaryOps::Add;
      case BinaryOperator::OpType::MINUS:
        return BinaryOps::Sub;
      case BinaryOperator::OpType::MULTIPLY:
        return BinaryOps::Mul;
      case BinaryOperator::OpType::REMAINDER:
        return BinaryOps::URem;
      default:
        unreachable("All arithmetic operator types handled above.");
    }
  };
  auto get_llvm_icmp_predicate = [](BinaryOperator::OpType op_type) {
    using Predicate = llvm::CmpInst::Predicate;
    switch (op_type) {
      case BinaryOperator::OpType::LESS:
        return Predicate::ICMP_SLT;
      case BinaryOperator::OpType::GREATER:
        return Predicate::ICMP_SGT;
      case BinaryOperator::OpType::LESS_EQ:
        return Predicate::ICMP_SLE;
      case BinaryOperator::OpType::GREATER_EQ:
        return Predicate::ICMP_SGE;
      case BinaryOperator::OpType::EQUALEQUAL:
        return Predicate::ICMP_EQ;
      case BinaryOperator::OpType::NOTEQUAL:
        return Predicate::ICMP_NE;
      default:
        unreachable("All comparison operator types handled above.");
    }
  };

  if (value.is_arithmetic()) {
    current_expr_value_ = llvm_ir_builder_->CreateBinOp(
        get_llvm_binary_op_type(value.op_type), left_op, right_op);
  } else {
    current_expr_value_ = llvm_ir_builder_->CreateICmp(
        get_llvm_icmp_predicate(value.op_type), left_op, right_op);
  }

  return true;
}

bool IRASTVisitor::traverse_function_declaration(const FunctionDecl& value) {
  const FunctionSymbolInfo& function_info = module_.functions_info.at(&value);
  llvm::Function* fun = get_or_insert_function(function_info);

  // external function doesn't have a body
  if (value.specifiers.is_extern()) {
    return true;
  }

  auto alloca_bb = llvm::BasicBlock::Create(llvm_context_, "alloca", fun);
  auto entry_bb = llvm::BasicBlock::Create(llvm_context_, "entry", fun);

  llvm_ir_builder_->SetInsertPoint(alloca_bb);
  allocate_local_variables(function_info);
  // here must be a br instruction, but we add it after all allocas are added

  llvm_ir_builder_->SetInsertPoint(entry_bb);
  for (size_t i = 0; i < value.parameters.size(); ++i) {
    traverse(*value.parameters[i]);

    // load parameter into allocated local variable
    llvm::Value* param_value = fun->getArg(i);
    llvm_ir_builder_->CreateStore(
        param_value, identifiers_addresses_[value.parameters[i].get()]);
  }

  // if function returns `unit` then we can create implicit return
  bool has_no_return = traverse(*value.body);
  if (has_no_return && value.return_type->value->is_unit()) {
    llvm_ir_builder_->CreateRetVoid();
  }

  // add br to basic block with allocas
  llvm_ir_builder_->SetInsertPoint(alloca_bb);
  llvm_ir_builder_->CreateBr(entry_bb);

  llvm::verifyFunction(*fun, &llvm::outs());
  identifiers_addresses_.clear();

  return true;
}

bool IRASTVisitor::traverse_return_statement(const ReturnStmt& value) {
  auto return_value = compile_expression(*value.value);
  llvm_ir_builder_->CreateRet(return_value);

  // we must stop generating IR for current scope after return statement
  return false;
}

bool IRASTVisitor::visit_integer_literal(const IntegerLiteral& value) {
  current_expr_value_ = llvm_ir_builder_->getInt64(value.value);
  return true;
}

bool IRASTVisitor::visit_bool_literal(const BoolLiteral& value) {
  current_expr_value_ = llvm_ir_builder_->getInt1(value.value);
  return true;
}

bool IRASTVisitor::traverse_member_expression(const MemberExpr& value) {
  llvm::Type* result_type = map_type(value.type);
  llvm::Value* cls_pointer = compile_expression(*value.left);
  llvm::Value* index = llvm_ir_builder_->getInt32(value.member_index);

  current_expr_value_ =
      llvm_ir_builder_->CreateInBoundsGEP(result_type, cls_pointer, {index});

  return true;
}

bool IRASTVisitor::traverse_tuple_expression(const TupleExpr& value) {
  llvm::Type* tuple_ty = map_type(value.type);

  llvm::IRBuilder temp_builder{llvm_context_};
  llvm::BasicBlock& alloca_bb =
      llvm_ir_builder_->GetInsertPoint()->getFunction()->getEntryBlock();
  temp_builder.SetInsertPoint(&alloca_bb);
  llvm::Value* tuple = temp_builder.CreateAlloca(tuple_ty);

  for (size_t i = 0; i < value.elements.size(); ++i) {
    Expression& element = *value.elements[i];
    llvm::Type* element_ty = map_type(element.type);
    llvm::Value* index = llvm_ir_builder_->getInt32(i);

    llvm::Value* element_ptr =
        llvm_ir_builder_->CreateInBoundsGEP(element_ty, tuple, {index});
    llvm_ir_builder_->CreateStore(compile_expression(element), element_ptr);
  }

  current_expr_value_ = tuple;
  return true;
}

bool IRASTVisitor::traverse_tuple_index_expression(
    const TupleIndexExpr& value) {
  llvm::Type* result_type = map_type(value.type);
  llvm::Value* tuple_ptr = compile_expression(*value.left);
  llvm::Value* index = llvm_ir_builder_->getInt32(value.index);

  current_expr_value_ =
      llvm_ir_builder_->CreateInBoundsGEP(result_type, tuple_ptr, {index});
  return true;
}

std::unique_ptr<llvm::Module> IRASTVisitor::compile() {
  traverse(*module_.ast_root);
  llvm::verifyModule(*llvm_module_, &llvm::errs());

  return std::move(llvm_module_);
}
}  // namespace Front
