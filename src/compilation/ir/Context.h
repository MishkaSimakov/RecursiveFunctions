#pragma once

namespace llvm {
class Module;
class LLVMContext;
class Value;
class BasicBlock;
class Type;
}  // namespace llvm

class StringPool;

namespace Front {

class Mangler;
class TypesStorage;

struct IRContext {
  llvm::Module& llvm_module;

  Mangler& mangler;
  StringPool& strings;
  TypesStorage& types;

  llvm::LLVMContext& get_llvm_context();
};

}  // namespace Front
