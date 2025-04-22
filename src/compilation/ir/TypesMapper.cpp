#include "TypesMapper.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#include "compilation/types/Type.h"
#include "errors/Helpers.h"

llvm::Type* Front::TypesMapper::operator()(Type* type) {
  auto& llvm_context = context_.get_llvm_context();

  type = type->get_original();
  switch (type->get_kind()) {
    case Type::Kind::SIGNED_INT:
    case Type::Kind::UNSIGNED_INT:
    case Type::Kind::CHAR: {
      auto* ptype = static_cast<PrimitiveType*>(type);
      return llvm::Type::getIntNTy(llvm_context, ptype->get_width());
    }
    case Type::Kind::BOOL:
      return llvm::Type::getInt1Ty(llvm_context);
    case Type::Kind::TUPLE: {
      TupleType* tuple_ty = static_cast<TupleType*>(type);

      auto mapped_range =
          tuple_ty->get_elements() | std::views::transform(*this);
      std::vector mapped(mapped_range.begin(), mapped_range.end());
      return llvm::StructType::get(llvm_context, mapped);
    }
    case Type::Kind::STRUCT: {
      StructType* class_ty = static_cast<StructType*>(type);
      // TODO: mangle!
      auto name = class_ty->get_name().to_string(context_.strings);
      auto mapped_range = class_ty->get_members() | std::views::values |
                          std::views::transform(*this);
      std::vector mapped(mapped_range.begin(), mapped_range.end());

      return llvm::StructType::create(llvm_context, mapped, name);
    }
    case Type::Kind::POINTER:
      return llvm::PointerType::get(llvm_context, 0);
    default:
      not_implemented();
  }
}
