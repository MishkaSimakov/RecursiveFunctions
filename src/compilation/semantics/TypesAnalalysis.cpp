#include "SemanticAnalyzer.h"

namespace Front {

bool SemanticAnalyzer::visit_pointer_type(PointerTypeNode& node) {
  node.value = types().add_pointer(node.child->value);
  return true;
}

bool SemanticAnalyzer::visit_primitive_type(PrimitiveTypeNode& node) {
  node.value = types().add_primitive(node.kind, node.width);
  return true;
}

bool SemanticAnalyzer::visit_tuple_type(TupleTypeNode& node) {
  size_t tuple_size = node.elements.size();

  std::vector<Type*> elements_types(tuple_size);
  for (size_t i = 0; i < tuple_size; ++i) {
    elements_types[i] = node.elements[i]->value;
  }

  node.value = types().make_type<TupleType>(std::move(elements_types));
  return true;
}

bool SemanticAnalyzer::visit_user_defined_type(UserDefinedTypeNode& node) {
  SymbolInfo* info = name_lookup(current_scope_, node.name);
  if (info == nullptr) {
    scold_user(node, "unknown type name");
  }

  Type* type = info->get_type();
  if (type == nullptr) {
    scold_user(node, "isn't a type");
  }

  node.value = type;

  return true;
}

}  // namespace Front
