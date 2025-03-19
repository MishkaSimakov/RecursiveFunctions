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
  ASSERT_FALSE(function.specifiers.is_exported());
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
  ASSERT_FALSE(function.specifiers.is_exported());
  ASSERT_EQ(function.parameters.size(), 2);
  ASSERT_TYPE_NODE_EQ(function.return_type, TypeKind::INT);

  auto& first_param = function.parameters[0];
  auto& second_param = function.parameters[1];

  ASSERT_STRING_EQ(first_param->name, "first");
  ASSERT_STRING_EQ(second_param->name, "second");

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
    auto& call_id = dynamic_cast<IdExpr&>(*call_expr.callee);

    ASSERT_ID_EQ(call_id, "function");

    ASSERT_TRUE(call_expr.arguments.empty());
  }

  {
    auto& statements = parse_function_body("math::sqrt(1234);");

    ASSERT_EQ(statements.size(), 1);

    auto& expr_stmt = dynamic_cast<ExpressionStmt&>(*statements.front());
    auto& call_expr = dynamic_cast<CallExpr&>(*expr_stmt.value);
    auto& call_id = dynamic_cast<IdExpr&>(*call_expr.callee);

    ASSERT_ID_EQ(call_id, "math", "sqrt");
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

TEST_F(SyntaxTestCase, minus_tests) {
  {
    auto& statements = parse_function_body("a - b;");

    auto& expr_stmt = dynamic_cast<ExpressionStmt&>(*statements.front());
    auto& binary_op = dynamic_cast<BinaryOperator&>(*expr_stmt.value);

    ASSERT_EQ(binary_op.op_type, BinaryOperator::OpType::MINUS);

    auto& left = dynamic_cast<IdExpr&>(*binary_op.left);
    auto& right = dynamic_cast<IdExpr&>(*binary_op.right);

    ASSERT_ID_EQ(left, "a");
    ASSERT_ID_EQ(right, "b");
  }

  {
    auto& statements = parse_function_body("-x;");

    auto& expr_stmt = dynamic_cast<ExpressionStmt&>(*statements.front());
    auto& unary_op = dynamic_cast<UnaryOperator&>(*expr_stmt.value);

    ASSERT_EQ(unary_op.op_type, UnaryOperator::OpType::MINUS);

    auto& value = dynamic_cast<IdExpr&>(*unary_op.value);
    ASSERT_ID_EQ(value, "x");
  }

  {
    auto& statements = parse_function_body("--x;");

    auto& expr_stmt = dynamic_cast<ExpressionStmt&>(*statements.front());
    auto& unary_op = dynamic_cast<UnaryOperator&>(*expr_stmt.value);

    ASSERT_EQ(unary_op.op_type, UnaryOperator::OpType::PREDECREMENT);

    auto& value = dynamic_cast<IdExpr&>(*unary_op.value);
    ASSERT_ID_EQ(value, "x");
  }

  {
    auto& statements = parse_function_body("x - --y;");

    auto& expr_stmt = dynamic_cast<ExpressionStmt&>(*statements.front());
    auto& binary_op = dynamic_cast<BinaryOperator&>(*expr_stmt.value);

    ASSERT_EQ(binary_op.op_type, BinaryOperator::OpType::MINUS);

    ASSERT_ID_EQ(dynamic_cast<IdExpr&>(*binary_op.left), "x");

    auto& right = dynamic_cast<UnaryOperator&>(*binary_op.right);
    ASSERT_EQ(right.op_type, UnaryOperator::OpType::PREDECREMENT);

    ASSERT_ID_EQ(dynamic_cast<IdExpr&>(*right.value), "y");
  }
}

TEST_F(SyntaxTestCase, test_nodes_range) {
  auto& context =
      parse("function: (x: i64) -> void = { if (x == 0) { call(); } }");

  auto& root = *context.ast_root;
  ASSERT_SOURCE_RANGE(root.source_range, 0, 56);

  auto& function = dynamic_cast<FunctionDecl&>(*root.declarations.front());
  ASSERT_SOURCE_RANGE(function.source_range, 0, 56);

  // check function arguments
  {
    auto& parameter = *function.parameters.front();
    ASSERT_SOURCE_RANGE(parameter.source_range, 11, 17);

    auto& parameter_type = *parameter.type;
    ASSERT_SOURCE_RANGE(parameter_type.source_range, 14, 17);
  }

  // check function body
  {
    auto& body = *function.body;
    ASSERT_SOURCE_RANGE(body.source_range, 29, 56);

    auto& if_stmt = static_cast<IfStmt&>(*body.statements.front());
    ASSERT_SOURCE_RANGE(if_stmt.condition->source_range, 35, 41);

    ASSERT_SOURCE_RANGE(if_stmt.true_branch->source_range, 43, 54);
    ASSERT_SOURCE_RANGE(if_stmt.false_branch->source_range, 54, 54);
  }
}
