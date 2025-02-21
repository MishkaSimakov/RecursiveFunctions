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

#define ASSERT_STRING_EQ(string_id, expected) \
  ASSERT_EQ(context_.get_string(string_id), expected)

#define ASSERT_TYPE_NODE_EQ(ast_node, expected) \
  ASSERT_EQ(ast_node->value->get_kind(), expected)

#define ASSERT_ID_EQ(id_node, ...) \
  ASSERT_TRUE(is_identifier_equal(id_node, __VA_ARGS__))

class SyntaxTestCase : public ::testing::Test {
 protected:
  GlobalContext context_;

  template <typename... Args>
  bool is_identifier_equal(const std::unique_ptr<IdExpr>& identifier,
                           const Args&... parts) {
    return is_identifier_equal(*identifier);
  }

  template <typename... Args>
  bool is_identifier_equal(const IdExpr& identifier, const Args&... parts) {
    std::vector<std::string_view> expected_parts{parts...};

    if (expected_parts.size() != identifier.parts.size()) {
      return false;
    }

    for (size_t i = 0; i < expected_parts.size(); ++i) {
      if (context_.get_string(identifier.parts[i]) != expected_parts[i]) {
        return false;
      }
    }

    return true;
  }

  const ModuleContext& parse(std::string_view program) {
    Lexis::LexicalAnalyzer lexical_analyzer(Constants::lexis_filepath);
    auto source_view = context_.source_manager.load_text(program);
    lexical_analyzer.set_source_view(source_view);

    auto& module_context =
        context_.add_module(std::to_string(context_.modules.size()));

    Syntax::LRParser parser(Constants::grammar_filepath, context_);
    parser.parse(lexical_analyzer, module_context.id);

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
