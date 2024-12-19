#include "RecursiveFunctionsGrammar.h"
#include "lr/LRTableSerializer.h"
#include "utils/Constants.h"

int main() {
  auto absolute_grammar_filepath =
      std::filesystem::path(BASE_PATH) / Constants::grammar_filepath;

  auto [builders, grammar] = Syntax::get_recursive_functions_grammar();
  auto builder = Syntax::LRTableBuilder(std::move(grammar));
  builder.save_to(absolute_grammar_filepath);
}
