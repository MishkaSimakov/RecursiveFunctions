#pragma once

namespace llvm {
class LLVMContext;
class Module;
class Function;
class BasicBlock;
class Type;
class Value;
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
