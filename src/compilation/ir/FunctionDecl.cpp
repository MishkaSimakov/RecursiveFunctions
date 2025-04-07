#include "FunctionDecl.h"

#include "compilation/SymbolInfo.h"

namespace Front {

bool IRFunctionDecl::return_through_argument() const {
  return !info_->type->return_type->is_passed_by_value();
}

}  // namespace Front
