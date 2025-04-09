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

}  // namespace Front
