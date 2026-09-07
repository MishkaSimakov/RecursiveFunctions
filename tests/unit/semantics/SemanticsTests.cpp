#include <gtest/gtest.h>
#include <llvm/IR/Module.h>

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

TEST_F(SemanticsTestCase, test_simple_function_info) {
  auto& module = analyze("f: () -> () = { x: i64 = 123; }");

  ASSERT_EQ(module.root_scope->symbols.size(), 1);

  StringId name = module.add_string("f");
  FunctionSymbolInfo& info =
      module.root_scope->symbols.at(name).as<FunctionSymbolInfo>();

  ASSERT_EQ(info.transformation_type, nullptr);
  ASSERT_EQ(info.name, name);
  ASSERT_EQ(info.scope, module.root_scope.get());

  // check names
  auto unqualified_name = info.get_unqualified_name();
  auto qualified_name = info.get_fully_qualified_name();

  ASSERT_EQ(unqualified_name, name);
  ASSERT_EQ(qualified_name, QualifiedId{name});

  // check subscope
  ASSERT_EQ(module.root_scope->children.size(), 1);

  Scope* subscope = module.root_scope->children.front().get();
  ASSERT_EQ(subscope, info.subscope);
}

TEST_F(SemanticsTestCase, test_function_variable_info) {
  auto& module =
      analyze("f: () -> () = { x: i64 = 123; y: i32 = 321 as! i32; }");

  StringId function_name = module.add_string("f");
  StringId xname = module.add_string("x");
  StringId yname = module.add_string("y");

  Scope* function_scope = module.root_scope->children.front().get();
  ASSERT_EQ(function_scope->symbols.size(), 2);

  VariableSymbolInfo& xinfo =
      function_scope->symbols.at(xname).as<VariableSymbolInfo>();
  VariableSymbolInfo& yinfo =
      function_scope->symbols.at(yname).as<VariableSymbolInfo>();

  ASSERT_EQ(xinfo.index, 0);
  ASSERT_EQ(yinfo.index, 1);

  ASSERT_EQ(xinfo.name, xname);
  ASSERT_EQ(yinfo.name, yname);

  ASSERT_EQ(xinfo.type, module.types_storage.add_primitive<SignedIntType>(64));
  ASSERT_EQ(yinfo.type, module.types_storage.add_primitive<SignedIntType>(32));

  ASSERT_EQ(xinfo.scope, function_scope);
  ASSERT_EQ(yinfo.scope, function_scope);

  // names
  auto xqualified_name = xinfo.get_fully_qualified_name();
  auto yqualified_name = yinfo.get_fully_qualified_name();

  ASSERT_EQ(xqualified_name, (QualifiedId{function_name, xname}));
  ASSERT_EQ(yqualified_name, (QualifiedId{function_name, yname}));

  auto xunqualified_name = xinfo.get_unqualified_name();
  auto yunqualified_name = yinfo.get_unqualified_name();

  ASSERT_EQ(xunqualified_name, xname);
  ASSERT_EQ(yunqualified_name, yname);
}

TEST_F(SemanticsTestCase, test_function_in_namespace_info) {
  auto& module = analyze("A: namespace = { f: () -> () = {} }");

  ASSERT_EQ(module.root_scope->symbols.size(), 1);

  StringId namespace_name = module.add_string("A");
  StringId function_name = module.add_string("f");

  Scope* namespace_scope = module.root_scope->children.front().get();
  ASSERT_EQ(namespace_scope, module.root_scope->symbols.at(namespace_name)
                                 .as<NamespaceSymbolInfo>()
                                 .subscope);

  FunctionSymbolInfo& info =
      namespace_scope->symbols.at(function_name).as<FunctionSymbolInfo>();

  ASSERT_EQ(info.transformation_type, nullptr);
  ASSERT_EQ(info.name, function_name);
  ASSERT_EQ(info.scope, namespace_scope);

  // check names
  auto unqualified_name = info.get_unqualified_name();
  auto qualified_name = info.get_fully_qualified_name();

  ASSERT_EQ(unqualified_name, function_name);
  ASSERT_EQ(qualified_name, (QualifiedId{namespace_name, function_name}));
}

TEST_F(SemanticsTestCase, test_parent_symbol_info) {
  auto& module =
      analyze("A: namespace = { f: () -> () = { { x: i64 = 123; } } }");

  // symbols: A -> f -> anonymous -> x
  Scope* namespace_scope = module.root_scope->children.front().get();
  SymbolInfo& namespace_info =
      module.root_scope->symbols.at(module.add_string("A"));
  ASSERT_EQ(namespace_scope->parent_symbol, &namespace_info);

  Scope* function_scope = namespace_scope->children.front().get();
  SymbolInfo& function_info = namespace_scope->symbols.at(module.add_string("f"));
  ASSERT_EQ(function_scope->parent_symbol, &function_info);

  Scope* anonymous_scope = function_scope->children.front().get();
  ASSERT_EQ(anonymous_scope->parent_symbol, nullptr);

  ASSERT_TRUE(anonymous_scope->symbols.contains(module.add_string("x")));
}
