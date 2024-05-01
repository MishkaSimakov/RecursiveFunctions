#ifndef PROGRAMRUNTESTCASE_H
#define PROGRAMRUNTESTCASE_H

#include <gtest/gtest.h>
#include <filesystem>

#include "RecursiveFunctions.h"

using Compilation::CompileTreeBuilder, Compilation::BytecodeCompiler;
using Preprocessing::Preprocessor, Preprocessing::TextSource,
    Preprocessing::FileSource;

class ProgramRunTestCase : public ::testing::Test {
 protected:
  static std::filesystem::path arithmetics_path_;
  static std::filesystem::path fast_arithmetics_path_;
  static std::filesystem::path is_prime_path_;

  ProgramRunTestCase() { Logger::disable_category(Logger::ALL); };

  static size_t run_program(string program, bool use_fast_arithmetics = false) {
    vector<string> program_with_includes = {};
    program_with_includes.push_back("#include \"arithmetics\"");
    program_with_includes.push_back("#include \"is_prime\"");
    program_with_includes.push_back(std::move(program));

    Preprocessing preprocessor;
    preprocessor.add_source<FileSource>(
        "arithmetics",
        use_fast_arithmetics ? fast_arithmetics_path_ : arithmetics_path_);
    preprocessor.add_source<FileSource>("is_prime", is_prime_path_);
    preprocessor.add_source<TextSource>("main",
                                        std::move(program_with_includes));

    string program_text = preprocessor.process();

    auto tokens = LexicalAnalyzer::get_tokens(program_text);

    auto syntax_tree = SyntaxTreeBuilder::build(
        tokens, RecursiveFunctionsSyntax::GetSyntax(),
        RecursiveFunctionsSyntax::RuleIdentifiers::PROGRAM);

    auto compile_tree_builder = CompileTreeBuilder();
    compile_tree_builder.add_internal_function("successor", 1);
    compile_tree_builder.add_internal_function("__add", 2);
    compile_tree_builder.add_internal_function("__abs_diff", 2);

    auto compile_tree = compile_tree_builder.build(*syntax_tree);

    BytecodeCompiler compiler;
    compiler.compile(*compile_tree);

    auto bytecode = compiler.get_result();

    BytecodeExecutor executor;
    ValueT result = executor.execute(bytecode);

    return result.as_value();
  }

  template <typename T>
    requires requires(T a) { string(a); }
  static string convert_to_string(T&& value) {
    return string(value);
  }

  template <typename T>
    requires requires(T a) { std::to_string(a); }
  static string convert_to_string(T&& value) {
    return std::to_string(value);
  }

  template <typename Head, typename... Tail>
  static string get_function_call(string name, Head&& head, Tail&&... tail) {
    string result = name + "(" + convert_to_string(head);

    if constexpr (sizeof...(tail) != 0) {
      result += (("," + convert_to_string(tail)) + ...);
    }

    return result + ");";
  }
};

inline std::filesystem::path ProgramRunTestCase::arithmetics_path_ =
    "programs/arithmetics.rec";
inline std::filesystem::path ProgramRunTestCase::fast_arithmetics_path_ =
    "programs/fast_arithmetics.rec";
inline std::filesystem::path ProgramRunTestCase::is_prime_path_ =
    "programs/is_prime.rec";

#endif  // PROGRAMRUNTESTCASE_H
