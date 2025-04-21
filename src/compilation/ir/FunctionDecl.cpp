#include "FunctionDecl.h"

#include <llvm/IR/Module.h>

#include "compilation/SymbolInfo.h"
#include "compilation/mangling/Mangler.h"
#include "compilation/types/TypesStorage.h"
#include "compilation/ir/TypesMapper.h"

namespace Front {

IRFunctionDecl IRFunctionDecl::create(IRContext context,
                                      const FunctionSymbolInfo& info) {
  std::string name = context.mangler.mangle(info);

  // create new function
  std::vector<llvm::Type*> arguments;
  bool return_through_arg = !info.type->get_return_type()->is_passed_by_value();
  size_t arguments_offset = 0;
  if (return_through_arg) {
    arguments_offset = 1;
    arguments.push_back(llvm::PointerType::get(context.get_llvm_context(), 0));
  }

  for (Type* argument_type : info.type->get_arguments()) {
    if (!argument_type->is_passed_by_value()) {
      argument_type = context.types.add_pointer(argument_type);
    }

    arguments.emplace_back(context.types_mapper(argument_type));
  }

  Type* ret_ty = info.type->get_return_type();
  llvm::Type* llvm_ret_ty;

  if (ret_ty->is_unit() || !ret_ty->is_passed_by_value()) {
    llvm_ret_ty = llvm::Type::getVoidTy(context.get_llvm_context());
  } else {
    llvm_ret_ty = context.types_mapper(ret_ty);
  }

  auto llvm_func_type = llvm::FunctionType::get(llvm_ret_ty, arguments, false);

  llvm::Function* fun =
      llvm::Function::Create(llvm_func_type, llvm::Function::ExternalLinkage,
                             name, context.llvm_module);

  if (return_through_arg) {
    fun->addParamAttr(
        0, llvm::Attribute::get(context.get_llvm_context(),
                                llvm::Attribute::StructRet,
                                context.types_mapper(info.type->get_return_type())));
    fun->getArg(0)->setName("result");
  }

  // set names for arguments
  auto& decl = static_cast<FunctionDecl&>(info.declaration);
  for (size_t i = 0; i < decl.parameters.size(); ++i) {
    fun->getArg(i + arguments_offset)
        ->setName(context.strings.get_string(decl.parameters[i]->name));
  }

  return IRFunctionDecl(fun, &info);
}

bool IRFunctionDecl::return_through_argument() const {
  return !info_->type->get_return_type()->is_passed_by_value();
}

}  // namespace Front
