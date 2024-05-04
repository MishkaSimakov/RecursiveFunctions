#ifndef MAIN_H
#define MAIN_H

#include <argparse/argparse.hpp>

#include "ExceptionsHandler.h"
#include "RecursiveFunctions.h"
#include "execution/debug/DebugBytecodeExecutor.h"

using Compilation::CompileTreeBuilder;
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
    return ExceptionsHandler::execute([argc, &argv] {
      argparse::ArgumentParser parser("interpeter");
      parser.add_description(
          "Interpreter for general recursive functions "
          "(https://en.wikipedia.org/wiki/General_recursive_function).");

      parser.add_argument("filepath").help("Path to main file.");
      parser.add_argument("-i", "--include")
          .nargs(argparse::nargs_pattern::any)
          .help(
              "adds include paths. You can write <name>:<filepath> to create "
              "implicitly named include, <filepath> to deduce include name "
              "automatically or <directory path> to include all files in "
              "directory recursively.");

      size_t verbosity = 0;
      parser.add_argument("-v")
          .action([&](const auto&) { ++verbosity; })
          .append()
          .default_value(false)
          .implicit_value(true)
          .nargs(0)
          .help(
              "increase program verbosity. -v - show only warnings, -vv - show "
              "info and "
              "warnings, -vvv - show all log messages.");

      parser.add_argument("-d", "--debug")
          .default_value(false)
          .implicit_value(true)
          .help("turn on debug mode");

      try {
        parser.parse_args(argc, argv);
      } catch (const std::exception& err) {
        throw ArgumentsParseException(err.what());
      }

      Logger::set_level(verbosity);

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
      Compilation::BytecodeCompiler compiler;
      compiler.compile(*compile_tree);

      auto bytecode = compiler.get_result();

      bool is_debug_enabled = parser.get<bool>("debug");
      if (is_debug_enabled) {
        DebugBytecodeExecutor executor(std::move(bytecode));
        executor.execute();

      } else {
        BytecodeExecutor executor;
        ValueT result = executor.execute(bytecode);

        cout << result.as_value() << endl;
      }
    });
  }
};
}  // namespace Cli

#endif  // MAIN_H
