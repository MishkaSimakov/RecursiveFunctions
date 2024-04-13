#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "compilation/BytecodePrinter.h"
#include "compilation/Compiler.h"
#include "execution/BytecodeExecutor.h"
#include "lexis/LexicalAnalyzer.h"
#include "preprocessor/Preprocessor.h"
#include "syntax/RecursiveFunctionsSyntax.h"
#include "syntax/buffalo/SyntaxTreeBuilder.h"

using namespace std;

template <class result_t = std::chrono::milliseconds,
          class clock_t = std::chrono::steady_clock,
          class duration_t = std::chrono::milliseconds>
auto since(std::chrono::time_point<clock_t, duration_t> const& start) {
  return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}

int main() {
  // setup logger
  Logger::disable_category(Logger::Category::ALL);

  std::filesystem::path base_path =
      "/Users/mihailsimakov/Documents/Programs/CLionProjects/"
      "RecursiveFunctions/tests";

  Preprocessor preprocessor;
  preprocessor.add_file("arithmetics", base_path / "fast_arithmetics.rec");
  preprocessor.add_file("is_prime", base_path / "is_prime.rec");

  preprocessor.add_file("test", base_path / "test.rec");
  preprocessor.set_main("test");
  string program_text = preprocessor.process();

  auto tokens = LexicalAnalyzer::get_tokens(program_text);

  auto syntax_tree = SyntaxTreeBuilder::build(
      tokens, RecursiveFunctionsSyntax::GetSyntax(),
      RecursiveFunctionsSyntax::RuleIdentifiers::PROGRAM);

  Compilation::BytecodeCompiler compiler;
  auto bytecode = compiler.compile(*syntax_tree);

  BytecodePrinter::print(bytecode);

  BytecodeExecutor executor;

  ValueT result = executor.execute(bytecode);

  cout << "Result: " << result.as_value() << endl;

  cout << "In overall execution took: "
       << executor.get_execution_duration().count() << "ms" << endl;

#if COLLECT_STATISTICS
  executor.print_statistics(cout);
#endif
}