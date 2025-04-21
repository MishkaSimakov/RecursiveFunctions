#include "ScopePrinter.h"

#include <fmt/format.h>

#include "utils/TupleUtils.h"

namespace Front {

void ScopePrinter::traverse_scope_recursively(const Scope& scope) {
  std::string name(strings_.get_string(scope.name));

  add_node(name);
  move_cursor_down();

  for (const auto& info : scope.symbols | std::views::values) {
    std::string qualified_name =
        info.get_fully_qualified_name().to_string(strings_);

    std::visit(
        Overloaded{[&](const FunctionSymbolInfo& fun) {
                     add_node(fmt::format("Fun {} {}", qualified_name,
                                          fun.type->to_string(strings_)));
                   },
                   [&](const VariableSymbolInfo& var) {
                     add_node(fmt::format("Var {} {}", qualified_name,
                                          var.type->to_string(strings_)));
                   },
                   [&](const NamespaceSymbolInfo& nmsp) {
                     add_node(fmt::format("Nmsp {}", qualified_name));
                   },
                   [&](const TypeAliasSymbolInfo& alias) {
                     add_node(fmt::format("Alias {}", qualified_name));
                   },
                   [&](const StructSymbolInfo& str) {
                     add_node(fmt::format("Struct {}", qualified_name));
                   }},
        info);
  }
  for (const auto& child : scope.children) {
    traverse_scope_recursively(*child);
  }

  move_cursor_up();
}

ScopePrinter::ScopePrinter(const StringPool& strings, const Scope& root_scope,
                           std::ostream& os)
    : TreePrinter(os), strings_(strings), root_scope_(root_scope) {}

void ScopePrinter::print() {
  move_cursor_down();
  traverse_scope_recursively(root_scope_);
  move_cursor_up();

  TreePrinter::print();
}

}  // namespace Front
