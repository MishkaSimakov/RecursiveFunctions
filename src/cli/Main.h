#pragma once

#include <argparse/argparse.hpp>

#include "ExceptionsHandler.h"
#include "RecursiveFunctions.h"
#include "assembly/AssemblyPrinter.h"
#include "intermediate_representation/compiler/IRCompiler.h"
#include "passes/PassManager.h"
#include "passes/branch_to_select/ReplaceBranchWithSelect.h"
#include "passes/common_elimination/CommonElimination.h"
#include "passes/constant_propagation/ConstantPropagationPass.h"
#include "passes/inline/InlinePass.h"
#include "passes/phi_elimination/PhiEliminationPass.h"
#include "passes/print/PrintPass.h"
#include "passes/recursion_to_loop/RecursionToLoopPass.h"
#include "passes/registers_allocation/RegisterAllocationPass.h"
#include "passes/silly_move_erasure/SSAMoveErasure.h"
#include "passes/silly_move_erasure/SillyMoveErasurePass.h"
#include "passes/unused_elimination/UnusedFunctionsEliminationPass.h"
#include "passes/unused_elimination/UnusedTemporariesEliminationPass.h"

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

    if (!fs::is_regular_file(main_filepath)) {
      throw std::runtime_error(fmt::format("No such file {}", main_filepath));
    }

    preprocessor.add_source<FileSource>("main", main_filepath);
    preprocessor.set_main_source("main");

    string separator{fs::path::preferred_separator};

    for (auto& include : includes) {
      if (include.contains(kIncludeNamePathDelimiter)) {
        // named include
        size_t index = include.find(kIncludeNamePathDelimiter);
        string name = include.substr(0, index);

        if (name.empty()) {
          throw std::runtime_error("Include name must be non-empty.");
        }

        fs::path path = include.substr(index + 1, include.size() - index);

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

        if (is_regular_file(path)) {
          fs::path path_copy = path;
          path_copy.replace_extension();

          auto include_name =
              std::regex_replace(string{path_copy}, std::regex(separator), ".");

          preprocessor.add_source<FileSource>(include_name, path);
          continue;
        }

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

      parser.add_argument("-o", "--output").help("specify output file name");

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
      auto compile_tree = compile_tree_builder.build(*syntax_tree);

      // generate intermediate representation
      auto ir = IR::IRCompiler().get_ir(*compile_tree);

      Passes::PassManager pass_manager(ir);

      pass_manager.register_pass<Passes::UnusedFunctionsEliminationPass>();
      pass_manager.register_pass<Passes::UnusedTemporariesEliminationPass>();

      pass_manager.register_pass<Passes::CommonElimination>();
      pass_manager.register_pass<Passes::RecursionToLoopPass>();

      pass_manager.register_pass<Passes::InlinePass>();

      pass_manager.register_pass<Passes::SSAMoveErasure>();
      pass_manager.register_pass<Passes::ReplaceBranchWithSelect>();

      pass_manager.register_pass<Passes::PhiEliminationPass>();

      pass_manager.register_pass<Passes::UnusedFunctionsEliminationPass>();
      pass_manager.register_pass<Passes::UnusedTemporariesEliminationPass>();


      auto config = Passes::PrintPassConfig{true, false};
  pass_manager.register_pass<Passes::PrintPass>(std::cout, config);

      pass_manager.register_pass<Passes::RegisterAllocationPass>();

      pass_manager.register_pass<Passes::SillyMoveErasurePass>();

      pass_manager.apply();
      //
      auto assembly = Assembly::AssemblyPrinter(ir).print();

      // TODO: add -o flag support
      if (parser.present("-o")) {
        throw std::runtime_error("not supported yet.");
      }

      auto temp_dir = fs::temp_directory_path();
      auto output_file = temp_dir / "somestupidassembly.s";

      std::ofstream file(output_file);

      for (auto line : assembly) {
        file << line << "\n";
        std::cout << line << "\n";
      }

      file.close();

      auto compile_command = fmt::format("g++ {} -o test", output_file.c_str());
      std::system(compile_command.c_str());
      std::system("./test");
    });
  }
};
}  // namespace Cli
