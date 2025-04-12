#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::visit_integer_literal(IntegerLiteral& node) {
  node.type = types().add_primitive<SignedIntType>(64);
  node.value_category = ValueCategory::RVALUE;
  return true;
}

bool SemanticAnalyzer::visit_string_literal(StringLiteral& node) {
  node.type = types().add_pointer(types().add_primitive<CharType>(8));
  node.value_category = ValueCategory::RVALUE;
  return true;
}

bool SemanticAnalyzer::visit_bool_literal(BoolLiteral& node) {
  node.type = types().add_primitive<BoolType>(8);
  node.value_category = ValueCategory::RVALUE;
  return true;
}
}  // namespace Front
