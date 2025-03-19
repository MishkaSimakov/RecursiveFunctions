#pragma once

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <lexis/LexicalAnalyzer.h>
#include <syntax/lr/LRParser.h>
#include <utils/Constants.h>

#include "compilation/GlobalContext.h"
#include "sources/SourceManager.h"

using namespace Front;
using TypeKind = Front::Type::Kind;

#define ASSERT_SOURCE_RANGE(range, expected_from, expected_to) \
  ASSERT_EQ(range.begin.pos_id, expected_from);                \
  ASSERT_EQ(range.end.pos_id, expected_to);

#define ASSERT_STRING_EQ(string_id, expected) \
  ASSERT_EQ(module().get_string(string_id), expected)

#define ASSERT_TYPE_NODE_EQ(ast_node, expected) \
  ASSERT_EQ(ast_node->value->get_kind(), expected)

#define ASSERT_ID_EQ(id_node, ...) \
  ASSERT_TRUE(is_identifier_equal(id_node, __VA_ARGS__))

class SyntaxTestCase : public ::testing::Test {
 private:
  GlobalContext* context_{nullptr};

 protected:
  const ModuleContext& module() { return context_->get_module("main"); }

  template <typename... Args>
  bool is_identifier_equal(const std::unique_ptr<IdExpr>& identifier,
                           const Args&... parts) {
    return is_identifier_equal(*identifier, parts...);
  }

  template <typename... Args>
  bool is_identifier_equal(const IdExpr& identifier, const Args&... parts) {
    std::vector<std::string_view> expected_parts{parts...};

    if (expected_parts.size() != identifier.id.parts.size()) {
      return false;
    }

    for (size_t i = 0; i < expected_parts.size(); ++i) {
      if (module().get_string(identifier.id.parts[i]) != expected_parts[i]) {
        return false;
      }
    }

    return true;
  }

  const ModuleContext& parse(std::string_view program) {
    // reset global context for each parse
    delete context_;
    context_ = new GlobalContext();

    Lexis::LexicalAnalyzer lexical_analyzer(Constants::lexis_filepath);
    auto source_view = context_->source_manager.load_text(program);
    lexical_analyzer.set_source_view(source_view);

    auto& module_context = context_->add_module("main");

    Syntax::LRParser parser(Constants::grammar_filepath);
    parser.parse(lexical_analyzer, module_context, source_view);

    return module_context;
  }

  const std::vector<std::unique_ptr<Statement>>& parse_function_body(
      std::string_view body) {
    std::string program = "function: () -> void = {";
    program += body;
    program += "}";

    const auto& context = parse(program);

    auto& function =
        dynamic_cast<FunctionDecl&>(*context.ast_root->declarations.front());
    return function.body->statements;
  }

  void TearDown() override {
    delete context_;
  }

  //
  // template <typename... Args>
  //   requires(std::same_as<Args, BinaryOperator::OpType> && ...)
  // bool check_operators_order(BinaryOperator& bin_op, Args... operators) {
  //   std::vector<BinaryOperator::OpType> order{operators...};
  //
  //   BinaryOperator* current = &bin_op;
  //   for (BinaryOperator::OpType op: order) {
  //     if (current->op_type != op) {
  //       return false;
  //     }
  //
  //     current = current->l
  //   }
  // }
};