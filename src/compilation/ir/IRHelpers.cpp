#include "IRGenerator.h"

namespace Front {

void IRGenerator::create_store(llvm::Value* destination, Value source,
                               Type* source_type) {
  if (source_type->is_unit()) {
    return;
  }

  if (source_type->get_kind() == Type::Kind::TUPLE) {
    auto* tuple_ty = static_cast<TupleType*>(source_type);
    create_tuple_like_copy(destination, source, tuple_ty);
  } else if (source_type->get_kind() == Type::Kind::STRUCT) {
    auto* class_ty = static_cast<StructType*>(source_type);
    create_tuple_like_copy(destination, source, class_ty);
  } else if (source_type->is_passed_by_value()) {
    source = remove_indirection(source, source_type);

    assert(!source.has_indirection);
    llvm_ir_builder_->CreateStore(source.llvm_value, destination);
  }
}

void IRGenerator::create_tuple_like_copy(llvm::Value* destination, Value source,
                                         TupleLikeType* source_type) {
  assert(source.has_indirection);

  size_t elements_count = source_type->get_elements_count();
  for (size_t i = 0; i < elements_count; ++i) {
    llvm::Value* zero = llvm_ir_builder_->getInt32(0);
    llvm::Value* index = llvm_ir_builder_->getInt32(i);
    llvm::Type* llvm_type = types_mapper_(source_type);

    llvm::Value* destination_element =
        llvm_ir_builder_->CreateGEP(llvm_type, destination, {zero, index});

    Value source_element;
    source_element.llvm_value = llvm_ir_builder_->CreateGEP(
        llvm_type, source.llvm_value, {zero, index});
    source_element.has_indirection = true;

    create_store(destination_element, source_element,
                 source_type->get_element_type(i));
  }
}

Value IRGenerator::remove_indirection(Value source, Type* type) {
  if (!source.has_indirection) {
    return source;
  }

  Value result;

  assert(type->is_passed_by_value());
  result.llvm_value =
      llvm_ir_builder_->CreateLoad(types_mapper_(type), source.llvm_value);
  result.has_indirection = false;

  return result;
}

}  // namespace Front
