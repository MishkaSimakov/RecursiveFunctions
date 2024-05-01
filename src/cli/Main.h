#ifndef MAIN_H
#define MAIN_H

#include <argparse/argparse.hpp>

#include "RecursiveFunctions.h"
#include "compilation/cpp/CppCompiler.h"

using Compilation::CompileTreeBuilder, Compilation::CppCompiler;
using Preprocessing::Preprocessor, Preprocessing::FileSource;

namespace Cli {
namespace fs = std::filesystem;

class Main {
  constexpr static auto kIncludeNamePathDelimiter = ":";

  static Preprocessor prepare_preprocessor(
      const argparse::ArgumentParser& parser) {
    Preprocessor preprocessor;

    auto includes = parser.get<vector<string>>("include");
    auto main_filepath = parser.get<string>("filepath");

    preprocessor.add_source<FileSource>("main", main_filepath);
    preprocessor.set_main_source("main");

    for (auto& include : includes) {
      if (include.contains(kIncludeNamePathDelimiter)) {
        // named include
        size_t index = include.find(kIncludeNamePathDelimiter);
        string name = include.substr(0, index);

        fs::path path = include.substr(index, include.size() - index);

        if (is_directory(path)) {
          throw std::runtime_error("Directory import can not be named.");
        }

        preprocessor.add_source<FileSource>(name, std::move(path));
      } else {
        // unnamed include
        // for this type of include name is stem part of path
        // directory can be passed as parameter to this type of include
        // if so all files will be included recursively
        // for example file with relative path from given directory root
        // foo/baz/prog.rec will be available with #include "foo.baz.prog"

        fs::path path = include;
        string separator{fs::path::preferred_separator};

        for (auto subfile : fs::recursive_directory_iterator(path)) {
          if (subfile.is_regular_file()) {
            string relative_path = relative(subfile, path).replace_extension();
            auto include_name =
                std::regex_replace(relative_path, std::regex(separator), ".");

            preprocessor.add_source<FileSource>(include_name, subfile.path());
          }
        }
      }
    }

    return preprocessor;
  }

 public:
  static int main(int argc, char* argv[]) {
    argparse::ArgumentParser parser("interpeter");
    parser.add_argument("filepath");
    parser.add_argument("-i", "--include").nargs(argparse::nargs_pattern::any);
    parser.add_argument("-v", "--verbose");
    // TODO: add verbose option

    try {
      parser.parse_args(argc, argv);
    } catch (const std::exception& err) {
      std::cerr << err.what() << std::endl;
      std::cerr << parser;
      return 1;
    }

    Logger::disable_category(Logger::Category::ALL);

    auto preprocessor = prepare_preprocessor(parser);
    string program_text = preprocessor.process();

    auto tokens = LexicalAnalyzer::get_tokens(program_text);

    auto syntax_tree = SyntaxTreeBuilder::build(
        tokens, RecursiveFunctionsSyntax::GetSyntax(),
        RecursiveFunctionsSyntax::RuleIdentifiers::PROGRAM);

    CompileTreeBuilder compile_tree_builder;
    compile_tree_builder.add_internal_function("successor", 1);
    compile_tree_builder.add_internal_function("__add", 2);
    compile_tree_builder.add_internal_function("__abs_diff", 2);

    auto compile_tree = compile_tree_builder.build(*syntax_tree);
    // Compilation::BytecodeCompiler compiler;
    // compiler.compile(*compile_tree);
    //
    // auto bytecode = compiler.get_result();
    //
    // BytecodeExecutor executor;
    // ValueT result = executor.execute(bytecode);
    //
    // cout << "Result: " << result.as_value() << endl;

    CppCompiler compiler;
    compiler.compile(*compile_tree);

    auto result = compiler.get_result();

    cout << result << endl;

    // auto temp_path = fs::temp_directory_path();
    // temp_path /= "program.cpp";
    //
    // std::ofstream temp_file(temp_path);
    // temp_file << result;
    // temp_file.close();
    //
    // std::system("g++ program.cpp -o program -O3");

    return 0;
  }
};
}  // namespace Cli

#endif  // MAIN_H
