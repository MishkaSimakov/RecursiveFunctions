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

  Syntax::GrammarGenerator::generate_grammar(
      input_filepath, absolute_grammar_filepath, builders_filepath);

  std::cout << "Stored grammar table in: " << absolute_grammar_filepath
            << std::endl;
}
