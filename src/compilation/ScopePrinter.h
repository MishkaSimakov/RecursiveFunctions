#pragma once

#include "ModuleContext.h"
#include "utils/Printing.h"
#include "utils/TupleUtils.h"

namespace Front {
class ScopePrinter : TreePrinter {
  const ModuleContext& context_;

  void traverse_scope_recursively(Scope& scope, bool is_root_scope) {
    std::string name;
    if (is_root_scope) {
      name = "module(" + context_.name + ")";
    } else if (scope.name.has_value()) {
      name = context_.get_string(scope.name.value());
    } else {
      name = std::format("anonymous({})", static_cast<void*>(&scope));
    }
    add_node(name);
    move_cursor_down();

    for (const auto& [name, info] : scope.symbols) {
      auto qualified_name = info.get_fully_qualified_name().parts |
                            std::views::transform([this](StringId part) {
                              return context_.get_string(part);
                            });

      std::visit(Overloaded{[&](const FunctionSymbolInfo& fun) {
                              add_node(fmt::format(
                                  "Fun {} {}", fmt::join(qualified_name, "::"),
                                  fun.type->to_string()));
                            },
                            [&](const VariableSymbolInfo& var) {
                              add_node(fmt::format(
                                  "Var {} {}", fmt::join(qualified_name, "::"),
                                  var.type->to_string()));
                            },
                            [&](const NamespaceSymbolInfo& nmsp) {
                              add_node(fmt::format(
                                  "Nmsp {}", fmt::join(qualified_name, "::")));
                            },
                            [&](const ExternalSymbolInfo& ext) {
                              add_node(fmt::format(
                                  "Ext {}", fmt::join(qualified_name, "::")));
                            }},
                 info);
    }
    for (const auto& child : scope.children) {
      traverse_scope_recursively(*child, false);
    }

    move_cursor_up();
  }

 public:
  explicit ScopePrinter(const ModuleContext& context, std::ostream& os)
      : TreePrinter(os), context_(context) {}

  void print() {
    move_cursor_down();
    traverse_scope_recursively(*context_.root_scope, true);
    move_cursor_up();

    TreePrinter::print();
  }
};
}  // namespace Front
