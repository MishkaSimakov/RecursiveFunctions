#pragma once

#include "utils/Printing.h"

namespace Front {
class ScopePrinter : TreePrinter {
  GlobalContext& context_;
  Scope& root_scope_;

  void traverse_scope_recursively(Scope& scope) {
    add_node(fmt::format("Scope  {}", static_cast<void*>(&scope)));

    move_cursor_down();

    for (auto [name, info] : scope.symbols) {
      add_node(fmt::format("Symbol {} {} {}", context_.get_string(name),
                           info.type->to_string(),
                           info.is_exported ? "exported" : "local"));
    }
    for (Scope* child : scope.children) {
      traverse_scope_recursively(*child);
    }

    move_cursor_up();
  }

 public:
  explicit ScopePrinter(std::ostream& os, GlobalContext& context,
                        Scope& root_scope)
      : TreePrinter(os), context_(context), root_scope_(root_scope) {}

  void print() {
    move_cursor_down();
    traverse_scope_recursively(root_scope_);
    move_cursor_up();

    TreePrinter::print();
  }
};
}  // namespace Front
