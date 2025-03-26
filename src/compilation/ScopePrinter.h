#pragma once

#include "ModuleContext.h"
#include "utils/Printing.h"

namespace Front {
class ScopePrinter : TreePrinter {
  const ModuleContext& context_;

  void traverse_scope_recursively(Scope& scope, bool is_root_scope);

 public:
  explicit ScopePrinter(const ModuleContext& context, std::ostream& os);

  void print();
};
}  // namespace Front
