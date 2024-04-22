#ifndef PROGRAMRUNTESTCASE_H
#define PROGRAMRUNTESTCASE_H

#include <gtest/gtest.h>

#include "RecursiveFunctions.h"

class ProgramRunTestCase : public ::testing::Test {
 protected:
  static std::filesystem::path arithmetics_path_;
  static std::filesystem::path fast_arithmetics_path_;
  static std::filesystem::path is_prime_path_;

  ProgramRunTestCase() = default;

  size_t RunProgram(string program, bool use_fast_arithmetics = false) {
    string program_with_includes =
        "#include \"arithmetics\"\n#include \"is_prime\"" + program;

    Preprocessor preprocessor;
    preprocessor.add_file("arithmetics", use_fast_arithmetics
                                             ? fast_arithmetics_path_
                                             : arithmetics_path_);
    preprocessor.add_file("is_prime", is_prime_path_);

    string program_text = preprocessor.process();

    auto tokens = LexicalAnalyzer::get_tokens(program);

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
  }
};

#endif  // PROGRAMRUNTESTCASE_H
