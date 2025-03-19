#pragma once

#include "utils/Printing.h"

namespace Front {
class ScopePrinter : TreePrinter {
  const ModuleContext& context_;

  void traverse_scope_recursively(Scope& scope) {
    add_node(fmt::format("Scope  {}", static_cast<void*>(&scope)));

    move_cursor_down();

    for (auto [name, info] : scope.symbols) {
      add_node(fmt::format("Symbol {} {} {}", context_.get_string(name),
                           info.type->to_string(),
                           info.is_exported ? "exported" : "local"));
    }
    for (const auto& child : scope.children) {
      traverse_scope_recursively(*child);
    }

    move_cursor_up();
  }

 public:
  explicit ScopePrinter(std::ostream& os, const ModuleContext& context)
      : TreePrinter(os), context_(context) {}

  void print() {
    move_cursor_down();
    traverse_scope_recursively(*context_.root_scope);
    move_cursor_up();

    TreePrinter::print();
  }
};
}  // namespace Front
