#pragma once

#include "ModuleContext.h"
#include "utils/Printing.h"

namespace Front {
class ScopePrinter : TreePrinter {
  const StringPool& strings_;
  const Scope& root_scope_;

  void traverse_scope_recursively(const Scope& scope);

 public:
  explicit ScopePrinter(const StringPool& strings, const Scope& root_scope,
                        std::ostream& os);

  void print();
};
}  // namespace Front
