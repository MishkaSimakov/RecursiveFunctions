#include "IRGenerator.h"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

namespace Front {

void IRGenerator::create_function_arguments() {
  const FunctionDecl& decl = current_function_->get_info()->get_decl();

  for (size_t i = 0; i < decl.parameters.size(); ++i) {
    VariableDecl& parameter = *decl.parameters[i];
    llvm::Value* llvm_arg = current_function_->get_llvm_argument(i);

    Type* arg_ty = parameter.type->value->get_original();

    llvm::Value* ptr;

    if (!arg_ty->is_passed_by_value()) {
      ptr = llvm_arg;
    } else {
      auto name = fmt::format("{}.addr", module_.get_string(decl.name));

      auto builder = get_alloca_builder();
      ptr = builder->CreateAlloca(types_mapper_(arg_ty), nullptr, name);
      builder->CreateStore(llvm_arg, ptr);
    }

    local_variables_.emplace(&parameter, ptr);
  }
}

llvm::Value* IRGenerator::get_local_variable_value(const VariableDecl& decl) {
  auto itr = local_variables_.find(&decl);

  if (itr == local_variables_.end()) {
    llvm::Value* ptr = get_alloca_builder()->CreateAlloca(
        types_mapper_(decl.type->value), nullptr,
        module_.get_string(decl.name));
    std::tie(itr, std::ignore) = local_variables_.emplace(&decl, ptr);
  }

  return itr->second;
}

IRFunctionDecl IRGenerator::get_or_insert_function(
    const FunctionSymbolInfo& info) {
  std::string name = mangler_.mangle(info);
  llvm::Function* function = llvm_module_->getFunction(name);
  if (function != nullptr) {
    return IRFunctionDecl(function, &info);
  }

  return IRFunctionDecl::create(get_context(), info);
}

std::unique_ptr<llvm::IRBuilder<>> IRGenerator::get_alloca_builder() {
  auto temp_builder = std::make_unique<llvm::IRBuilder<>>(llvm_context_);

  assert(current_function_.has_value());

  temp_builder->SetInsertPoint(current_function_->get_alloca_block());
  return temp_builder;
}

llvm::Value* IRGenerator::get_slot(Type* type) {
  return get_alloca_builder()->CreateAlloca(types_mapper_(type));
}

IRContext IRGenerator::get_context() {
  return IRContext{*llvm_module_, mangler_, module_.get_strings_pool(),
                   module_.types_storage, types_mapper_};
}

IRGenerator::IRGenerator(llvm::LLVMContext& llvm_context, ModuleContext& module)
    : llvm_context_(llvm_context),
      llvm_module_(std::make_unique<llvm::Module>(module.name, llvm_context_)),
      llvm_ir_builder_(std::make_unique<llvm::IRBuilder<>>(llvm_context_)),
      module_(module),
      mangler_(module.get_strings_pool()),
      types_mapper_(get_context()) {}

bool IRGenerator::traverse_implicit_lvalue_to_rvalue_conversion_expression(
    const ImplicitLvalueToRvalueConversionExpr& value) {
  current_expr_value_ = compile_expr(value.value);
  return true;
}

bool IRGenerator::traverse_if_statement(const IfStmt& value) {
  Value condition_value =
      remove_indirection(compile_expr(value.condition), value.condition->type);
  llvm::Function* current_function = current_function_->get_llvm_function();

  auto* true_branch =
      llvm::BasicBlock::Create(llvm_context_, "true_br", current_function);
  auto* false_branch = llvm::BasicBlock::Create(llvm_context_, "false_br");
  auto* merge = llvm::BasicBlock::Create(llvm_context_, "ifcont");

  llvm_ir_builder_->CreateCondBr(condition_value.llvm_value, true_branch,
                                 false_branch);

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
  Value condition_value =
      remove_indirection(compile_expr(value.condition), value.condition->type);
  llvm_ir_builder_->CreateCondBr(condition_value.llvm_value, loop_block,
                                 after_loop_block);

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
  current_expr_value_ = compile_call_expression(value);
  return true;
}

bool IRGenerator::traverse_id_expression(const IdExpr& value) {
  current_expr_value_ = compile_id_expression(value);
  return true;
}

bool IRGenerator::traverse_binary_operator(const BinaryOperator& value) {
  current_expr_value_ = compile_binary_operator(value);
  return true;
}

bool IRGenerator::traverse_function_declaration(const FunctionDecl& value) {
  FunctionSymbolInfo& info = module_.functions_info.at(&value);
  IRFunctionDecl decl = get_or_insert_function(info);

  // external function doesn't have a body
  if (value.specifiers.is_extern()) {
    return true;
  }

  auto alloca_bb = llvm::BasicBlock::Create(llvm_context_, "alloca",
                                            decl.get_llvm_function());
  auto entry_bb = llvm::BasicBlock::Create(llvm_context_, "entry",
                                           decl.get_llvm_function());
  auto return_bb = llvm::BasicBlock::Create(llvm_context_, "return",
                                            decl.get_llvm_function());

  llvm_ir_builder_->SetInsertPoint(alloca_bb);
  llvm::Value* result_ptr;
  bool return_through_arg = decl.return_through_argument();

  if (info.type->return_type->is_unit()) {
    result_ptr = nullptr;
  } else if (return_through_arg) {
    result_ptr = decl.get_llvm_function()->getArg(0);
  } else {
    result_ptr = llvm_ir_builder_->CreateAlloca(
        types_mapper_(value.return_type->value), nullptr, "result");
  }

  current_function_ = IRFunction(decl, alloca_bb, return_bb, result_ptr);

  create_function_arguments();

  llvm_ir_builder_->SetInsertPoint(entry_bb);
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
  if (return_through_arg || info.type->return_type->is_unit()) {
    llvm_ir_builder_->CreateRetVoid();
  } else {
    auto result =
        llvm_ir_builder_->CreateLoad(types_mapper_(value.return_type->value),
                                     current_function_->get_return_value());
    llvm_ir_builder_->CreateRet(result);
  }

  llvm::verifyFunction(*decl.get_llvm_function(), &llvm::outs());

  // cleanup
  local_variables_.clear();

  current_function_ = std::nullopt;

  return true;
}

bool IRGenerator::traverse_return_statement(const ReturnStmt& value) {
  Value result = compile_expr(value.value);
  create_store(current_function_->get_return_value(), result,
               value.value->type);
  llvm_ir_builder_->CreateBr(current_function_->get_return_block());

  // we must stop generating IR for current scope after return statement
  return false;
}

bool IRGenerator::traverse_variable_declaration(const VariableDecl& value) {
  if (value.initializer != nullptr) {
    llvm::Value* var_ptr = get_local_variable_value(value);
    Value initializer_value = compile_expr(value.initializer);

    create_store(var_ptr, initializer_value, value.initializer->type);
  }

  return true;
}

bool IRGenerator::traverse_assignment_statement(const AssignmentStmt& value) {
  Value right_value = compile_expr(value.right);
  Value left_value = compile_expr(value.left);

  assert(left_value.has_indirection);

  create_store(left_value.llvm_value, right_value, value.right->type);

  return true;
}

bool IRGenerator::traverse_member_expression(const MemberExpr& value) {
  current_expr_value_ = compile_member_expression(value);
  return true;
}

bool IRGenerator::traverse_implicit_tuple_copy_expression(
    const ImplicitTupleCopyExpr& value) {
  current_expr_value_ = compile_expr(value.value);
  return true;
}

bool IRGenerator::traverse_unary_operator(const UnaryOperator& value) {
  current_expr_value_ = compile_unary_operator(value);
  return true;
}

bool IRGenerator::traverse_explicit_unsafe_cast_expression(
    const ExplicitUnsafeCastExpr& value) {
  current_expr_value_ = compile_explicit_unsafe_cast(value);
  return true;
}

bool IRGenerator::traverse_tuple_expression(const TupleExpr& value) {
  current_expr_value_ = compile_tuple_expression(value);
  return true;
}

bool IRGenerator::visit_integer_literal(const IntegerLiteral& value) {
  current_expr_value_ = compile_integer_literal(value);
  return true;
}

bool IRGenerator::visit_bool_literal(const BoolLiteral& value) {
  current_expr_value_ = compile_bool_literal(value);
  return true;
}

bool IRGenerator::visit_string_literal(const StringLiteral& value) {
  current_expr_value_ = compile_string_literal(value);
  return true;
}

bool IRGenerator::traverse_tuple_index_expression(const TupleIndexExpr& value) {
  current_expr_value_ = compile_tuple_index_expression(value);
  return true;
}

std::unique_ptr<llvm::Module> IRGenerator::compile() {
  traverse(*module_.ast_root);
  llvm::verifyModule(*llvm_module_, &llvm::errs());

  return std::move(llvm_module_);
}
}  // namespace Front
