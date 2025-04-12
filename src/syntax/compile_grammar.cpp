#include <fmt/core.h>

#include "grammar/GrammarGenerator.h"
#include "utils/Constants.h"

int main() {
  auto grammar_filepath = Constants::GetBuildFilePath("grammar/grammar.lr");

  auto input_filepath = std::filesystem::path(GRAMMAR_TABLEGEN_TEXT_INPUT);
  auto builders_filepath =
      std::filesystem::path(GRAMMAR_TABLEGEN_BUILDERS_OUTPUT);

  size_t states_count = Syntax::GrammarGenerator::generate_grammar(
      input_filepath, grammar_filepath, builders_filepath);

  fmt::print(
      "Successfully generated grammar table with {} states. Stored in {:?}.\n",
      states_count, grammar_filepath.c_str());
}
