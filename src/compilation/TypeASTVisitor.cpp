#include "TypeASTVisitor.h"
w
Scope* TypeASTVisitor::local_symbol_lookup(size_t index) const {
  Scope* scope = current_scope_;

  while (scope != nullptr) {
    auto itr = scope->symbols.find(index);

    if (itr != scope->symbols.end()) {
      return scope;
    }

    scope = scope->parent;
  }

  return nullptr;
}

//
// bool TypeASTVisitor::traverse_function_declaration(FunctionDecl& value) {
//   // create new scope
//   visit_compound_statement(*value.body);
//
//   // add arguments into that scope
//   for (auto& argument : value.parameters) {
//     traverse(*argument);
//   }
//
//   // traverse body
//   traverse(*value.body);
// }
//
bool TypeASTVisitor::visit_id_expression(const IdExpr& value) {
  Scope* scope = local_symbol_lookup(value.id);
  value.type =
}
