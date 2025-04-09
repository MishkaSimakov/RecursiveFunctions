#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

#include "IRGenerator.h"

namespace Front {

void IRGenerator::create_store(llvm::Value* destination, Value source,
                               Type* source_type) {
  if (source_type->is_unit()) {
    return;
  }

  if (source_type->get_kind() == Type::Kind::TUPLE) {
    TupleType* tuple_ty = static_cast<TupleType*>(source_type);
    create_tuple_copy_constructor(destination, source, tuple_ty);
  } else if (source_type->is_passed_by_value()) {
    source = remove_indirection(source, source_type);

    assert(!source.has_indirection);
    llvm_ir_builder_->CreateStore(source.llvm_value, destination);
  } else {
    not_implemented("copy constructor for classes");
  }
}

void IRGenerator::create_tuple_copy_constructor(llvm::Value* destination,
                                                Value source,
                                                TupleType* source_type) {
  assert(source.has_indirection);

  std::vector<llvm::Type*> types{
      llvm_ir_builder_->getPtrTy(),
      llvm_ir_builder_->getPtrTy(),
      llvm_ir_builder_->getInt64Ty(),
  };

  auto size =
      llvm_ir_builder_->getInt64(llvm_module_->getDataLayout().getTypeAllocSize(
          types_mapper_(source_type)));

  llvm::Function* memcpy_fun = llvm::Intrinsic::getDeclaration(
      llvm_module_.get(), llvm::Intrinsic::memcpy, std::move(types));

  llvm_ir_builder_->CreateCall(
      memcpy_fun,
      {destination, source.llvm_value, size, llvm_ir_builder_->getInt1(false)});
}

Value IRGenerator::remove_indirection(Value source, Type* type) {
  if (!source.has_indirection) {
    return source;
  }

  Value result;

  result.llvm_value =
      llvm_ir_builder_->CreateLoad(types_mapper_(type), source.llvm_value);
  result.has_indirection = false;

  return result;
}

Value IRGenerator::compile_expr(const std::unique_ptr<Expression>& expr) {
  current_expr_value_ = Value::invalid();
  traverse(*expr);
  assert(current_expr_value_ != Value::invalid() &&
         "Expression value must be calculated.");
  return std::exchange(current_expr_value_, Value::invalid());
}

Value IRGenerator::compile_tuple_expression(const TupleExpr& value) {
  Value result;

  result.llvm_value = get_slot(value.type);
  result.has_indirection = true;

  for (size_t i = 0; i < value.elements.size(); ++i) {
    llvm::Value* zero = llvm_ir_builder_->getInt32(0);
    llvm::Value* index = llvm_ir_builder_->getInt32(i);

    llvm::Value* element_ptr = llvm_ir_builder_->CreateGEP(
        types_mapper_(value.type), result.llvm_value, {zero, index});
    create_store(element_ptr, compile_expr(value.elements[i]),
                 value.elements[i]->type);
  }

  return result;
}

Value IRGenerator::compile_member_expression(const MemberExpr& value) {
  Value class_value = compile_expr(value.left);
  assert(class_value.has_indirection);

  return llvm_ir_builder_->CreateGEP(
      types_mapper_(value.type), class_value.llvm_value,
      llvm_ir_builder_->getInt32(value.member_index));
}

Value IRGenerator::compile_binary_operator(const BinaryOperator& value) {
  Value left_value =
      remove_indirection(compile_expr(value.left), value.left->type);
  Value right_value =
      remove_indirection(compile_expr(value.right), value.left->type);

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

  Value result;

  if (value.is_arithmetic()) {
    result.llvm_value = llvm_ir_builder_->CreateBinOp(
        get_llvm_binary_op_type(value.op_type), left_value.llvm_value,
        right_value.llvm_value);
  } else {
    result.llvm_value = llvm_ir_builder_->CreateICmp(
        get_llvm_icmp_predicate(value.op_type), left_value.llvm_value,
        right_value.llvm_value);
  }

  result.has_indirection = false;

  return result;
}

Value IRGenerator::compile_unary_operator(const UnaryOperator& value) {
  Value result;
  Value argument_value =
      remove_indirection(compile_expr(value.value), value.value->type);

  if (value.op_type == UnaryOperator::OpType::NOT) {
    result.llvm_value =
        llvm_ir_builder_->CreateXor(argument_value.llvm_value, 1);
  } else {
    not_implemented("unary operator");
  }

  result.has_indirection = false;
  return result;
}

Value IRGenerator::compile_id_expression(const IdExpr& value) {
  const SymbolInfo& info = module_.symbols_info.at(&value);

  Value result;

  if (info.is_function()) {
    const FunctionSymbolInfo& fun = std::get<FunctionSymbolInfo>(info);
    result.llvm_value = get_or_insert_function(fun).get_llvm_function();
  } else if (info.is_variable()) {
    const VariableSymbolInfo& var = std::get<VariableSymbolInfo>(info);
    result.llvm_value = get_local_variable_value(var.get_decl());
  } else {
    unreachable("SemanticAnalyzer must throw if any other type appears here");
  }

  result.has_indirection = true;

  return result;
}

Value IRGenerator::compile_tuple_index_expression(const TupleIndexExpr& value) {
  llvm::Type* tuple_ty = types_mapper_(value.left->type);

  Value tuple = compile_expr(value.left);
  assert(tuple.has_indirection);

  llvm::Value* zero = llvm_ir_builder_->getInt32(0);
  llvm::Value* index = llvm_ir_builder_->getInt32(value.index);

  Value result;

  result.llvm_value =
      llvm_ir_builder_->CreateGEP(tuple_ty, tuple.llvm_value, {zero, index});
  result.has_indirection = true;

  return result;
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

Value IRGenerator::compile_call_expression(const CallExpr& value) {
  FunctionType* fun_ty = static_cast<FunctionType*>(value.callee->type);

  Value callee_value = compile_expr(value.callee);
  assert(callee_value.has_indirection);

  llvm::Function* llvm_callee =
      llvm::dyn_cast<llvm::Function>(callee_value.llvm_value);
  std::vector<llvm::Value*> arguments;
  bool return_through_arg = !fun_ty->return_type->is_passed_by_value();

  if (return_through_arg) {
    arguments.push_back(get_slot(fun_ty->return_type));
  }

  for (auto& argument : value.arguments) {
    Type* arg_ty = argument->type->get_original();
    Value argument_value = compile_expr(argument);

    if (arg_ty->is_passed_by_value()) {
      argument_value = remove_indirection(argument_value, arg_ty);
    } else {
      assert(argument_value.has_indirection);
    }

    arguments.push_back(argument_value.llvm_value);
  }

  Value result;

  llvm::Value* call_result =
      llvm_ir_builder_->CreateCall(llvm_callee, arguments);

  if (return_through_arg) {
    result.llvm_value = arguments[0];
    result.has_indirection = true;
  } else {
    result.llvm_value = call_result;
    result.has_indirection = false;
  }

  return result;
}

Value IRGenerator::compile_integer_literal(const IntegerLiteral& value) {
  Value result;

  result.llvm_value = llvm_ir_builder_->getInt64(value.value);
  result.has_indirection = false;

  return result;
}

Value IRGenerator::compile_bool_literal(const BoolLiteral& value) {
  Value result;

  result.llvm_value = llvm_ir_builder_->getInt1(value.value);
  result.has_indirection = false;

  return result;
}

}  // namespace Front
