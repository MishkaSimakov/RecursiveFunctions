#include <gtest/gtest.h>
#include <lexis/LexicalAnalyzer.h>
#include <syntax/lr/LRParser.h>
#include <utils/Constants.h>

#include <sstream>
#include <vector>

#include "compilation/GlobalContext.h"
#include "interpretation/ASTInterpreter.h"

std::vector<int64_t> run(std::string_view program) {
  Front::GlobalContext context;
  auto& source_manager = context.source_manager;

  Lexis::LexicalAnalyzer lexical_analyzer(Constants::lexis_filepath);
  auto parser = Syntax::LRParser(Constants::grammar_filepath);

  SourceView source_view = source_manager.load_text(program);
  lexical_analyzer.set_source_view(source_view);

  auto& module_context = context.add_module("main");

  parser.parse(lexical_analyzer, module_context, source_view);

  std::stringstream output_stream;
  Interpretation::ASTInterpreter(module_context, output_stream).interpret();

  auto output = output_stream.str();

  // remove last \n
  output.pop_back();

  auto splitted =
      output | std::views::split('\n') | std::views::transform([](auto&& part) {
        int64_t result{};

        auto [_, ec] =
            std::from_chars(part.data(), part.data() + part.size(), result);

        if (ec == std::errc()) {
          return result;
        }
        throw std::runtime_error("Not a number printed by interpreter.");
      });
  return std::vector(splitted.begin(), splitted.end());
}

TEST(InterpreterTests, basic_programs_tests) {
  {
    auto program = "main: () -> void = { print(2 + 2 * 2); }";
    auto result = run(program);

    auto expected = std::vector<int64_t>{6};
    ASSERT_EQ(result, expected);
  }

  {
    auto program = "main: () -> void = { a: i64 = 123; if (a == 123) { print(0); } else { print(1); } print(a + 321); }";
    auto result = run(program);

    auto expected = std::vector<int64_t>{0, 444};
    ASSERT_EQ(result, expected);
  }

  {
    auto program = "main: () -> void = { a: bool = (1 == 2); if (a) { print(0); } print(1); }";
    auto result = run(program);

    auto expected = std::vector<int64_t>{1};
    ASSERT_EQ(result, expected);
  }

  {
    auto program = "main: () -> void = { a: i64; a = 123; if (a == 123) { print(0); } a = 321; if (a == 321) { print(1); } }";
    auto result = run(program);

    auto expected = std::vector<int64_t>{0, 1};
    ASSERT_EQ(result, expected);
  }

  {
    auto program = "main: () -> void = { a: i64; if (2 + 2 == 4) { a = 1; } else { a = 0; } print(a); }";
    auto result = run(program);

    auto expected = std::vector<int64_t>{1};
    ASSERT_EQ(result, expected);
  }
}
