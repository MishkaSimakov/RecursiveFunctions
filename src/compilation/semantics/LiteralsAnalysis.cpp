#include "SemanticAnalyzer.h"

namespace Front {
bool SemanticAnalyzer::before_integer_literal(IntegerLiteral& node) {
  node.type = types().add_primitive<IntType>();
  node.value_category = ValueCategory::RVALUE;
  return true;
}

bool SemanticAnalyzer::before_string_literal(StringLiteral& node) {
  node.type = types().add_pointer(types().add_primitive<CharType>());
  node.value_category = ValueCategory::RVALUE;
  return true;
}

bool SemanticAnalyzer::before_bool_literal(BoolLiteral& node) {
  node.type = types().add_primitive<BoolType>();
  node.value_category = ValueCategory::RVALUE;
  return true;
}
}  // namespace Front
