#include <gtest/gtest.h>

#include <vector>

#include "SyntaxTestCase.h"

TEST_F(SyntaxTestCase, empty_program_test) {
  auto& context = parse("");

  auto& root = *context.ast_root;
  ASSERT_TRUE(root.declarations.empty());
  ASSERT_TRUE(root.imports.empty());
}

TEST_F(SyntaxTestCase, simple_function_test) {
  auto& context = parse(R"(
    function: () -> void = {}
  )");

  auto& root = *context.ast_root;
  ASSERT_TRUE(root.imports.empty());

  ASSERT_EQ(root.declarations.size(), 1);
  auto& function = dynamic_cast<FunctionDecl&>(*root.declarations.front());

  ASSERT_STRING_EQ(function.name, "function");
  ASSERT_FALSE(function.is_exported);
  ASSERT_TRUE(function.parameters.empty());
  ASSERT_TYPE_NODE_EQ(function.return_type, TypeKind::VOID);

  auto& body = dynamic_cast<CompoundStmt&>(*function.body);

  ASSERT_TRUE(body.statements.empty());
}

TEST_F(SyntaxTestCase, function_with_parameters) {
  auto& context = parse(R"(
    function: (first: i64, second: bool) -> i64 = {}
  )");

  auto& root = *context.ast_root;
  ASSERT_TRUE(root.imports.empty());

  ASSERT_EQ(root.declarations.size(), 1);
  auto& function = dynamic_cast<FunctionDecl&>(*root.declarations.front());

  ASSERT_STRING_EQ(function.name, "function");
  ASSERT_FALSE(function.is_exported);
  ASSERT_EQ(function.parameters.size(), 2);
  ASSERT_TYPE_NODE_EQ(function.return_type, TypeKind::BOOL);

  auto& first_param = function.parameters[0];
  auto& second_param = function.parameters[1];

  ASSERT_STRING_EQ(first_param->id, "first");
  ASSERT_STRING_EQ(second_param->id, "second");

  ASSERT_TYPE_NODE_EQ(first_param->type, TypeKind::INT);
  ASSERT_TYPE_NODE_EQ(second_param->type, TypeKind::BOOL);

  auto& body = dynamic_cast<CompoundStmt&>(*function.body);

  ASSERT_TRUE(body.statements.empty());
}

TEST_F(SyntaxTestCase, function_call_test) {
  {
    auto& statements = parse_function_body("function();");

    ASSERT_EQ(statements.size(), 1);

    auto& expr_stmt = dynamic_cast<ExpressionStmt&>(*statements.front());
    auto& call_expr = dynamic_cast<CallExpr&>(*expr_stmt.value);

    ASSERT_ID_EQ(call_expr.name, "function");
    ASSERT_TRUE(call_expr.arguments.empty());
  }

  {
    auto& statements = parse_function_body("math::sqrt(1234);");

    ASSERT_EQ(statements.size(), 1);

    auto& expr_stmt = dynamic_cast<ExpressionStmt&>(*statements.front());
    auto& call_expr = dynamic_cast<CallExpr&>(*expr_stmt.value);

    ASSERT_ID_EQ(call_expr.name, "math", "sqrt");
    ASSERT_EQ(call_expr.arguments.size(), 1);

    auto& argument =
        dynamic_cast<IntegerLiteral&>(*call_expr.arguments.front());
    ASSERT_EQ(argument.value, 1234);
  }
}

TEST_F(SyntaxTestCase, expression_test) {
  {
    auto& statements = parse_function_body("a + b;");

    auto& expr_stmt = dynamic_cast<ExpressionStmt&>(*statements.front());
    auto& binary_op = dynamic_cast<BinaryOperator&>(*expr_stmt.value);

    ASSERT_EQ(binary_op.op_type, BinaryOperator::OpType::PLUS);

    auto& left = dynamic_cast<IdExpr&>(*binary_op.left);
    auto& right = dynamic_cast<IdExpr&>(*binary_op.right);

    ASSERT_ID_EQ(left, "a");
    ASSERT_ID_EQ(right, "b");
  }
}

