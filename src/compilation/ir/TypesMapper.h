#pragma once

#include "Context.h"

namespace Front {
struct Type;

class TypesMapper {
  IRContext context_;

 public:
  explicit TypesMapper(IRContext context) : context_(context) {}

  llvm::Type* operator()(Type* type);
};

}  // namespace Front
