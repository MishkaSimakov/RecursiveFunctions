#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "compilation/BytecodePrinter.h"
#include "compilation/CompileTreeBuilder.h"
#include "compilation/bytecode/BytecodeCompiler.h"
#include "execution/BytecodeExecutor.h"
#include "lexis/LexicalAnalyzer.h"
#include "preprocessor/FileSource.h"
#include "preprocessor/Preprocessor.h"
#include "preprocessor/TextSource.h"
#include "syntax/RecursiveFunctionsSyntax.h"
#include "syntax/buffalo/SyntaxTreeBuilder.h"

using namespace std;
using Compilation::CompileTreeBuilder;
using Preprocessing::Preprocessor, Preprocessing::FileSource,
    Preprocessing::TextSource;

int main() {
  // setup logger
  Logger::disable_category(Logger::Category::ALL);

  std::filesystem::path base_path =
      "/Users/mihailsimakov/Documents/Programs/CLionProjects/"
      "RecursiveFunctions/examples";

  Preprocessor preprocessor;
  preprocessor.add_source<FileSource>("arithmetics",
                                      base_path / "fast_arithmetics.rec");
  preprocessor.add_source<FileSource>("is_prime", base_path / "is_prime.rec");
  preprocessor.add_source<FileSource>("test", base_path / "test.rec");
  preprocessor.set_main_source("test");

  string program_text = preprocessor.process();

  cout << program_text << endl;

  auto tokens = LexicalAnalyzer::get_tokens(program_text);

  auto syntax_tree = SyntaxTreeBuilder::build(
      tokens, RecursiveFunctionsSyntax::GetSyntax(),
      RecursiveFunctionsSyntax::RuleIdentifiers::PROGRAM);

  CompileTreeBuilder compile_tree_builder;
  compile_tree_builder.add_internal_function("successor", 1);
  // compile_tree_builder.add_internal_function("__add", 2);
  // compile_tree_builder.add_internal_function("__abs_diff", 2);

  auto compile_tree = compile_tree_builder.build(*syntax_tree);
  Compilation::BytecodeCompiler compiler;
  compiler.compile(*compile_tree);

  auto bytecode = compiler.get_result();
  BytecodePrinter::print(bytecode);

  BytecodeExecutor executor;
  ValueT result = executor.execute(bytecode);

  cout << "Result: " << result.as_value() << endl;
  //
  //   cout << "In overall execution took: "
  //        << executor.get_execution_duration().count() << "ms" << endl;
  //
  // #if COLLECT_STATISTICS
  //   executor.print_statistics(cout);
  // #endif
}