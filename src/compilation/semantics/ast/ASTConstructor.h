#pragma once

#include "ast/Nodes.h"
#include "compilation/types/Type.h"

namespace Front {

class ASTConstructor {
 public:
  static std::unique_ptr<TypeNode> create_type_node(Type* type,
                                                    SourceRange source_range);
};

}  // namespace Front
