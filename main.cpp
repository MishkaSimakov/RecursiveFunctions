#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "compilation/BytecodePrinter.h"
#include "compilation/Compiler.h"
#include "execution/BytecodeExecutor.h"
#include "lexis/LexicalAnalyzer.h"
#include "preprocessor/Preprocessor.h"
#include "syntax/AdditionSyntax.h"
#include "syntax/RecursiveFunctionsSyntax.h"
#include "syntax/buffalo/SyntaxTreeBuilder.h"

using namespace std;

int main() {
  // setup logger
  Logger::disable_category(Logger::Category::ALL);

  std::filesystem::path base_path =
      "/Users/mihailsimakov/Documents/Programs/CLionProjects/"
      "RecursiveFunctions/tests";

  Preprocessor preprocessor;
  preprocessor.add_file("arithmetics", base_path / "arithmetics.rec");
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

  std::cout << "Executed successfully, result: " << result.as_value() << endl;
}