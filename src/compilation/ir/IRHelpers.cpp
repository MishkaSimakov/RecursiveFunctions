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

  for (size_t i = 0; i < source_type->elements.size(); ++i) {
    llvm::Value* zero = llvm_ir_builder_->getInt32(0);
    llvm::Value* index = llvm_ir_builder_->getInt32(i);
    llvm::Type* llvm_type = types_mapper_(source_type);

    llvm::Value* destination_element =
        llvm_ir_builder_->CreateGEP(llvm_type, destination, {zero, index});

    Value source_element;
    source_element.llvm_value = llvm_ir_builder_->CreateGEP(
        llvm_type, source.llvm_value, {zero, index});
    source_element.has_indirection = true;

    create_store(destination_element, source_element, source_type->elements[i]);
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
