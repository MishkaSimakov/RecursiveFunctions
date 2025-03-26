#pragma once

#include <fmt/format.h>
#include <gtest/gtest.h>

#include <ranges>

#include "lexis/LexicalAnalyzer.h"
#include "utils/Constants.h"

class LexisTestCase : public ::testing::Test {
 protected:
  static Lexis::LexicalAnalyzer setup_analyzer(std::string_view program) {
    SourceView source_view(program, SourceLocation{0, 0});
    Lexis::LexicalAnalyzer analyzer(
        Constants::GetRuntimeFilePath(Constants::lexis_relative_filepath));
    analyzer.set_source_view(source_view);

    return analyzer;
  }

  static void test_string(std::string_view program,
                          const std::vector<Lexis::TokenType>& tokens) {
    auto analyzer = setup_analyzer(program);

    for (auto expected_type : tokens) {
      auto current_token = analyzer.next_token();
      ASSERT_EQ(current_token.type, expected_type);
    }
  }

  static void test_sequence(
      const std::vector<std::pair<std::string_view, Lexis::TokenType>>& parts) {
    std::string program;
    for (auto part : parts | std::views::keys) {
      program += part;
    }

    auto analyzer = setup_analyzer(program);

    size_t offset = 0;
    for (auto [part, expected_type] : parts) {
      size_t prev_offset = offset;
      offset += part.size();

      if (expected_type == Lexis::TokenType::WHITESPACE) {
        continue;
      }

      // test current_token + get_token
      for (size_t i = 0; i < 5; ++i) {
        auto token = i == 0 ? analyzer.next_token() : analyzer.current_token();
        ASSERT_EQ(token.type, expected_type)
            << fmt::format("Token types don't match. Expected {}, got {}",
                           expected_type.to_string(), token.type.to_string());

        ASSERT_EQ(token.source_range.begin.pos_id, prev_offset);
        ASSERT_EQ(token.source_range.end.pos_id, offset);
      }
    }

    // the last one should be END
    auto end_token = analyzer.next_token();
    ASSERT_EQ(end_token.type, Lexis::TokenType::END)
        << "Expected END token in the end. Got: " << end_token.type.to_string();
  }
};
