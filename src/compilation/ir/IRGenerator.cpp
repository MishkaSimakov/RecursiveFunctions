#include "IRGenerator.h"

#include <llvm/IR/Verifier.h>

namespace Front {
llvm::Type* IRGenerator::map_type(Type* type) const {
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

llvm::Value* IRGenerator::compile_expr(
    const std::unique_ptr<Expression>& expr) {
  current_expr_value_ = nullptr;
  traverse(*expr);
  assert(current_expr_value_ != nullptr &&
         "Expression value must be calculated.");
  return std::exchange(current_expr_value_, nullptr);
}

void IRGenerator::compile_expr_to(llvm::Value* variable,
                                  const std::unique_ptr<Expression>& expr) {
  current_expr_value_ = nullptr;
  // slot_ = variable;
  // traverse(*expr);
  // slot_ = nullptr;

  llvm::Value* result = compile_expr(expr);
  llvm_ir_builder_->CreateStore(result, variable);
  current_expr_value_ = nullptr;
}

void IRGenerator::create_function_arguments(const FunctionSymbolInfo& info,
                                            llvm::Function* llvm_fun) {
  const auto& decl = static_cast<const FunctionDecl&>(info.declaration);
  for (size_t i = 0; i < decl.parameters.size(); ++i) {
    VariableDecl& parameter = *decl.parameters[i];

    LocalVariableInfo parameter_info;

    llvm::Value* llvm_arg = llvm_fun->getArg(i);
    Type* arg_ty = parameter.type->value->get_original();
    parameter_info.original_type = map_type(arg_ty);

    if (!arg_ty->is_passed_by_value()) {
      parameter_info.has_indirection = true;
      parameter_info.pointer = llvm_arg;
    } else {
      auto name = fmt::format("{}.addr", module_.get_string(decl.name));

      parameter_info.has_indirection = false;
      auto builder = get_alloca_builder();
      parameter_info.pointer =
          builder->CreateAlloca(map_type(arg_ty), nullptr, name);
      builder->CreateStore(llvm_arg, parameter_info.pointer);
    }

    local_variables_.emplace(&parameter, parameter_info);
  }
}

llvm::Value* IRGenerator::get_local_variable_value(const VariableDecl& decl) {
  auto itr = local_variables_.find(&decl);

  if (itr == local_variables_.end()) {
    LocalVariableInfo info;
    info.original_type = map_type(decl.type->value);
    info.pointer = get_alloca_builder()->CreateAlloca(
        info.original_type, nullptr, module_.get_string(decl.name));
    info.has_indirection = false;
    std::tie(itr, std::ignore) = local_variables_.emplace(&decl, info);
  }

  LocalVariableInfo& info = itr->second;
  return info.pointer;
}

llvm::Function* IRGenerator::get_or_insert_function(
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
  llvm::Type* llvm_ret_ty;

  if (ret_ty->is_unit()) {
    llvm_ret_ty = llvm_ir_builder_->getVoidTy();
  } else {
    if (!ret_ty->is_passed_by_value()) {
      ret_ty = module_.types_storage.add_pointer(ret_ty);
    }

    llvm_ret_ty = map_type(ret_ty);
  }

  auto llvm_func_type = llvm::FunctionType::get(llvm_ret_ty, arguments, false);

  llvm::Function* fun =
      llvm::Function::Create(llvm_func_type, llvm::Function::ExternalLinkage,
                             name, llvm_module_.get());

  // set names for arguments
  auto& decl = static_cast<FunctionDecl&>(info.declaration);
  for (size_t i = 0; i < decl.parameters.size(); ++i) {
    fun->getArg(i)->setName(module_.get_string(decl.parameters[i]->name));
  }

  return fun;
}

std::unique_ptr<llvm::IRBuilder<>> IRGenerator::get_alloca_builder() {
  auto temp_builder = std::make_unique<llvm::IRBuilder<>>(llvm_context_);

  assert(current_function_.has_value());

  temp_builder->SetInsertPoint(current_function_->get_alloca_block());
  return temp_builder;
}

IRContext IRGenerator::get_context() {
  return IRContext{*llvm_module_, mangler_, module_.get_strings_pool(),
                   module_.types_storage};
}

IRGenerator::IRGenerator(llvm::LLVMContext& llvm_context, ModuleContext& module)
    : llvm_context_(llvm_context),
      llvm_module_(std::make_unique<llvm::Module>(module.name, llvm_context_)),
      llvm_ir_builder_(std::make_unique<llvm::IRBuilder<>>(llvm_context_)),
      mangler_(module.get_strings_pool()),
      module_(module) {}

bool IRGenerator::traverse_implicit_lvalue_to_rvalue_conversion_expression(
    const ImplicitLvalueToRvalueConversionExpr& value) {
  llvm::Value* ptr = compile_expr(value.value);
  current_expr_value_ = llvm_ir_builder_->CreateLoad(map_type(value.type), ptr);
  return true;
}

bool IRGenerator::traverse_assignment_statement(const AssignmentStmt& value) {
  compile_expr_to(compile_expr(value.left), value.right);
  return true;
}

bool IRGenerator::traverse_if_statement(const IfStmt& value) {
  llvm::Value* condition = compile_expr(value.condition);
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

bool IRGenerator::traverse_while_statement(const WhileStmt& value) {
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
  llvm::Value* condition = compile_expr(value.condition);
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

bool IRGenerator::traverse_call_expression(const CallExpr& value) {
  llvm::Function* callee =
      llvm::dyn_cast<llvm::Function>(compile_expr(value.callee));

  std::vector<llvm::Value*> arguments;
  arguments.reserve(value.arguments.size());
  for (auto& argument : value.arguments) {
    Type* arg_ty = argument->type->get_original();

    if (arg_ty->is_passed_by_value()) {
      arguments.emplace_back(compile_expr(argument));
    } else {
      // create temporary
      llvm::Value* tmp_ptr =
          get_alloca_builder()->CreateAlloca(map_type(arg_ty));
      compile_expr_to(tmp_ptr, argument);
      arguments.emplace_back(tmp_ptr);
    }
  }

  current_expr_value_ = llvm_ir_builder_->CreateCall(callee, arguments);
  return true;
}

bool IRGenerator::traverse_variable_declaration(const VariableDecl& value) {
  if (value.initializer != nullptr) {
    llvm::Value* var_ptr = get_local_variable_value(value);
    compile_expr_to(var_ptr, value.initializer);
  }

  return true;
}

bool IRGenerator::traverse_id_expression(const IdExpr& value) {
  const SymbolInfo& info = module_.symbols_info.at(&value);

  std::visit(
      Overloaded{
          [&](const auto&) {
            unreachable(
                "SemanticAnalyzer ensures that other options are impossible.");
          },
          [&](const VariableSymbolInfo& var) {
            auto& decl = static_cast<const VariableDecl&>(var.declaration);
            current_expr_value_ = get_local_variable_value(decl);
          },
          [&](const FunctionSymbolInfo& fun) {
            current_expr_value_ = get_or_insert_function(fun);
          }},
      info);

  return true;
}

bool IRGenerator::traverse_binary_operator(const BinaryOperator& value) {
  auto left_op = compile_expr(value.left);
  auto right_op = compile_expr(value.right);

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

bool IRGenerator::traverse_function_declaration(const FunctionDecl& value) {
  const FunctionSymbolInfo& function_info = module_.functions_info.at(&value);
  llvm::Function* fun = get_or_insert_function(function_info);

  // external function doesn't have a body
  if (value.specifiers.is_extern()) {
    return true;
  }

  auto alloca_bb = llvm::BasicBlock::Create(llvm_context_, "alloca", fun);
  auto entry_bb = llvm::BasicBlock::Create(llvm_context_, "entry", fun);
  auto return_bb = llvm::BasicBlock::Create(llvm_context_, "return", fun);

  llvm_ir_builder_->SetInsertPoint(alloca_bb);
  llvm::Value* result_ptr = llvm_ir_builder_->CreateAlloca(
      map_type(value.return_type->value), nullptr, "result");

  current_function_ = IRFunction(alloca_bb, return_bb, result_ptr);

  // swap
  llvm_ir_builder_->SetInsertPoint(entry_bb);
  create_function_arguments(function_info, fun);

  for (size_t i = 0; i < value.parameters.size(); ++i) {
    VariableDecl& parameter = *value.parameters[i];
    // for now default arguments values are not supported
    // traverse(*value.parameters[i]);
    if (parameter.initializer != nullptr) {
      not_implemented("default function arguments");
    }
  }

  // if function returns `unit` then we can create implicit return
  bool has_no_return = traverse(*value.body);
  if (has_no_return && value.return_type->value->is_unit()) {
    llvm_ir_builder_->CreateBr(current_function_->get_return_block());
  }

  // add br to basic block with allocas
  llvm_ir_builder_->SetInsertPoint(alloca_bb);
  llvm_ir_builder_->CreateBr(entry_bb);

  llvm_ir_builder_->SetInsertPoint(return_bb);
  auto result =
      llvm_ir_builder_->CreateLoad(map_type(value.return_type->value),
                                   current_function_->get_return_value());
  llvm_ir_builder_->CreateRet(result);

  llvm::verifyFunction(*fun, &llvm::outs());

  // cleanup
  local_variables_.clear();

  current_function_ = std::nullopt;

  return true;
}

bool IRGenerator::traverse_return_statement(const ReturnStmt& value) {
  compile_expr_to(current_function_->get_return_value(), value.value);
  // llvm::Value* result = compile_expr(value.value);
  // llvm_ir_builder_->CreateStore(result,
  // current_function_->get_return_value());
  llvm_ir_builder_->CreateBr(current_function_->get_return_block());

  // we must stop generating IR for current scope after return statement
  return false;
}

bool IRGenerator::visit_integer_literal(const IntegerLiteral& value) {
  current_expr_value_ = llvm_ir_builder_->getInt64(value.value);

  // llvm_ir_builder_->CreateStore(llvm_ir_builder_->getInt64(value.value),
  // current_initializing_value_);
  return true;
}

bool IRGenerator::visit_bool_literal(const BoolLiteral& value) {
  current_expr_value_ = llvm_ir_builder_->getInt1(value.value);
  return true;
}

bool IRGenerator::traverse_member_expression(const MemberExpr& value) {
  llvm::Type* result_type = map_type(value.type);
  llvm::Value* cls_pointer = compile_expr(value.left);
  llvm::Value* index = llvm_ir_builder_->getInt32(value.member_index);

  current_expr_value_ =
      llvm_ir_builder_->CreateGEP(result_type, cls_pointer, {index});

  return true;
}

bool IRGenerator::traverse_tuple_expression(const TupleExpr& value) {
  llvm::Type* tuple_ty = map_type(value.type);
  llvm::Value* tuple = get_alloca_builder()->CreateAlloca(tuple_ty);

  for (size_t i = 0; i < value.elements.size(); ++i) {
    llvm::Value* zero = llvm_ir_builder_->getInt32(0);
    llvm::Value* index = llvm_ir_builder_->getInt32(i);

    llvm::Value* element_ptr =
        llvm_ir_builder_->CreateGEP(tuple_ty, tuple, {zero, index});
    compile_expr_to(element_ptr, value.elements[i]);
  }

  current_expr_value_ = tuple;
  return true;
}

bool IRGenerator::traverse_tuple_index_expression(const TupleIndexExpr& value) {
  llvm::Type* tuple_ty = map_type(value.left->type);
  llvm::Value* tuple_ptr = compile_expr(value.left);
  llvm::Value* zero = llvm_ir_builder_->getInt32(0);
  llvm::Value* index = llvm_ir_builder_->getInt32(value.index);

  current_expr_value_ =
      llvm_ir_builder_->CreateGEP(tuple_ty, tuple_ptr, {zero, index});
  return true;
}

bool IRGenerator::traverse_implicit_tuple_copy_expression(
    const ImplicitTupleCopyExpr& value) {
  std::vector<llvm::Type*> types{
      llvm_ir_builder_->getPtrTy(),
      llvm_ir_builder_->getPtrTy(),
      llvm_ir_builder_->getInt64Ty(),
  };

  llvm::Type* tuple_ty = map_type(value.type);
  auto size = llvm_ir_builder_->getInt64(
      llvm_module_->getDataLayout().getTypeAllocSize(tuple_ty));

  llvm::Function* memcpy_fun = llvm::Intrinsic::getDeclaration(
      llvm_module_.get(), llvm::Intrinsic::memcpy, std::move(types));

  llvm::Value* dest_ptr;
  if (slot_ != nullptr) {
    dest_ptr = slot_;
  } else {
    dest_ptr = get_alloca_builder()->CreateAlloca(map_type(value.type));
  }

  llvm::Value* source_ptr = compile_expr(value.value);

  llvm_ir_builder_->CreateCall(memcpy_fun, {dest_ptr, source_ptr, size,
                                            llvm_ir_builder_->getInt1(false)});
  current_expr_value_ = dest_ptr;
  return true;
}

bool IRGenerator::traverse_unary_operator(const UnaryOperator& value) {
  if (value.op_type == UnaryOperator::OpType::NOT) {
    llvm::Value* argument = compile_expr(value.value);
    llvm::Value* truth = llvm_ir_builder_->getInt1(true);
    current_expr_value_ = llvm_ir_builder_->CreateXor(argument, truth);
  } else {
    not_implemented("unary operator");
  }

  return true;
}

std::unique_ptr<llvm::Module> IRGenerator::compile() {
  traverse(*module_.ast_root);
  llvm::verifyModule(*llvm_module_, &llvm::errs());

  return std::move(llvm_module_);
}
}  // namespace Front
