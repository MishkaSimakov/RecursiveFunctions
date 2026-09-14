#include <fmt/core.h>

#include "grammar/GrammarGenerator.h"

int main() {
  auto input_filepath = std::filesystem::path(GRAMMAR_TABLEGEN_TEXT_INPUT);
  auto builders_filepath =
      std::filesystem::path(GRAMMAR_TABLEGEN_BUILDERS_OUTPUT);
  auto grammar_filepath = std::filesystem::path(GRAMMAR_TABLEGEN_LR_OUTPUT);

  size_t states_count = Syntax::GrammarGenerator::generate_grammar(
      input_filepath, grammar_filepath, builders_filepath);

  fmt::println("Successfully generated grammar table with {} states.",
               states_count);

  fmt::println("LR table stored in: {:?}.", grammar_filepath.c_str());
  fmt::println("AST builders registry stored in: {:?}.",
               builders_filepath.c_str());
}
