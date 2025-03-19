#include <fmt/core.h>
#include <iostream>

#include "grammar/GrammarGenerator.h"
#include "lr/LRTableSerializer.h"
#include "utils/Constants.h"

int main() {
  auto absolute_grammar_filepath =
      std::filesystem::path(BASE_PATH) / Constants::grammar_filepath;

  auto input_filepath =
      std::filesystem::path(BASE_PATH) / "src" / "syntax" / "grammar.txt";
  auto builders_filepath = std::filesystem::path(BASE_PATH) / "src" / "syntax" /
                           "BuildersRegistry.h";

  size_t states_count = Syntax::GrammarGenerator::generate_grammar(
      input_filepath, absolute_grammar_filepath, builders_filepath);

  fmt::print(
      "Successfully generated grammar table with {} states. Stored in {:?}.\n",
      states_count, absolute_grammar_filepath.c_str());
}
