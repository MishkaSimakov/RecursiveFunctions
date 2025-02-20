#pragma once

#include <fmt/format.h>
#include <gtest/gtest.h>

#include <ranges>

#include "lexis/LexicalAnalyzer.h"
#include "utils/Constants.h"

class LexisTestCase : public ::testing::Test {
 protected:
  static void test_string(std::string_view program,
                          const std::vector<Lexis::TokenType>& tokens) {
    SourceManager source_manager;
    auto begin = source_manager.load_text(program);

    Lexis::LexicalAnalyzer analyzer(Constants::lexis_filepath, source_manager);
    analyzer.set_location(begin);

    for (auto expected_type : tokens) {
      auto current_token = analyzer.get_token();
      ASSERT_EQ(current_token.type, expected_type);
    }
  }

  static void test_sequence(
      const std::vector<std::pair<std::string_view, Lexis::TokenType>>& parts) {
    std::string program;
    for (auto part : parts | std::views::keys) {
      program += part;
    }

    SourceManager source_manager;
    auto begin = source_manager.load_text(program);

    Lexis::LexicalAnalyzer analyzer(Constants::lexis_filepath, source_manager);
    analyzer.set_location(begin);

    size_t offset = 0;
    for (auto [part, expected_type] : parts) {
      size_t prev_offset = offset;
      offset += part.size();

      if (expected_type == Lexis::TokenType::WHITESPACE) {
        continue;
      }

      auto token = analyzer.get_token();
      ASSERT_EQ(token.type, expected_type)
          << fmt::format("Token types don't match. Expected {}, got {}",
                         expected_type.to_string(), token.type.to_string());

      ASSERT_EQ(token.source_range.begin.pos_id, prev_offset);
      ASSERT_EQ(token.source_range.end.pos_id, offset);
    }

    // the last one should be END
    auto end_token = analyzer.get_token();
    ASSERT_EQ(end_token.type, Lexis::TokenType::END)
        << "Expected END token in the end. Got: " << end_token.type.to_string();
  }
};
