#include <gtest/gtest.h>

#include <vector>

#include "SemanticsTestCase.h"

TEST_F(SemanticsTestCase, explicit_unsafe_cast_expression) {
  auto [scope, statements] = analyze_function("a: i32 = 123 as! i32;");

  // check expression type and value category
  auto& var_decl = dynamic_cast<VariableDecl&>(
      *dynamic_cast<DeclarationStmt&>(*statements[0]).value);
  auto& cast_expr =
      dynamic_cast<ExplicitUnsafeCastExpr&>(*var_decl.initializer);

  auto& type = dynamic_cast<SignedIntType&>(*cast_expr.type);
  ASSERT_EQ(type.get_width(), 32);
  ASSERT_EQ(cast_expr.value_category, ValueCategory::RVALUE);
}
