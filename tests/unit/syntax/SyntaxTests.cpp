#include <gtest/gtest.h>

#include <vector>

#include "SyntaxTestCase.h"

using namespace Front;
using TypeKind = Front::Type::Kind;

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
