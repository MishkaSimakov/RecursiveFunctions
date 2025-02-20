#pragma once

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <lexis/LexicalAnalyzer.h>
#include <syntax/lr/LRParser.h>
#include <utils/Constants.h>

#include "compilation/GlobalContext.h"
#include "sources/SourceManager.h"

#define ASSERT_STRING_EQ(string_id, expected) \
  ASSERT_EQ(context_.get_string(string_id), expected)

#define ASSERT_TYPE_NODE_EQ(ast_node, expected) \
  ASSERT_EQ(ast_node->value->get_kind(), expected)

class SyntaxTestCase : public ::testing::Test {
 protected:
  Front::GlobalContext context_;

  const Front::ModuleContext& parse(std::string_view program) {
    Lexis::LexicalAnalyzer lexical_analyzer(Constants::lexis_filepath);
    auto source_view = context_.source_manager.load_text(program);
    lexical_analyzer.set_source_view(source_view);

    auto& module_context =
        context_.add_module(std::to_string(context_.modules.size()));

    Syntax::LRParser parser(Constants::grammar_filepath, context_);
    parser.parse(lexical_analyzer, module_context.id);

    return module_context;
  }
};
