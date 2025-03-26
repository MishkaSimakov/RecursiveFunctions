#include "ScopePrinter.h"

#include <fmt/format.h>

#include "utils/TupleUtils.h"

namespace Front {

void ScopePrinter::traverse_scope_recursively(Scope& scope,
                                              bool is_root_scope) {
  std::string name = is_root_scope ? "module(" + context_.name + ")"
                                   : std::string(context_.get_string(scope.name));

  add_node(name);
  move_cursor_down();

  for (const auto& [name, info] : scope.symbols) {
    auto qualified_name = info.get_fully_qualified_name().parts |
                          std::views::transform([this](StringId part) {
                            return context_.get_string(part);
                          });

    std::visit(
        Overloaded{
            [&](const FunctionSymbolInfo& fun) {
              add_node(fmt::format("Fun {} {}", fmt::join(qualified_name, "::"),
                                   fun.type->to_string()));
            },
            [&](const VariableSymbolInfo& var) {
              add_node(fmt::format("Var {} {}", fmt::join(qualified_name, "::"),
                                   var.type->to_string()));
            },
            [&](const NamespaceSymbolInfo& nmsp) {
              add_node(fmt::format("Nmsp {}", fmt::join(qualified_name, "::")));
            }},
        info);
  }
  for (const auto& child : scope.children) {
    traverse_scope_recursively(*child, false);
  }

  move_cursor_up();
}

ScopePrinter::ScopePrinter(const ModuleContext& context, std::ostream& os)
    : TreePrinter(os), context_(context) {}

void ScopePrinter::print() {
  move_cursor_down();
  traverse_scope_recursively(*context_.root_scope, true);
  move_cursor_up();

  TreePrinter::print();
}

}  // namespace Front
