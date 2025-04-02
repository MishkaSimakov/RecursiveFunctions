#include "SemanticAnalyzer.h"

namespace Front {

bool SemanticAnalyzer::visit_tuple_expression(TupleExpr& node) {
  size_t tuple_size = node.elements.size();

  std::vector<Type*> elements_types(tuple_size);
  for (size_t i = 0; i < tuple_size; ++i) {
    convert_to_rvalue(node.elements[i]);
    elements_types[i] = node.elements[i]->type;
  }

  node.type = types().make_type<TupleType>(std::move(elements_types));
  node.value_category = ValueCategory::RVALUE;

  return true;
}

}  // namespace Front
