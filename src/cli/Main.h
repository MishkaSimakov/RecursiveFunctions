#pragma once

#include <argparse/argparse.hpp>

#include "ArgumentsReader.h"
#include "ExceptionsHandler.h"
#include "assembly/AssemblyPrinter.h"
#include "compilation/CompileTreeBuilder.h"
#include "filesystem/FilesystemManager.h"
#include "intermediate_representation/IRPrinter.h"
#include "intermediate_representation/compiler/IRCompiler.h"
#include "lexis/LexicalAnalyzer.h"
#include "passes/PassManager.h"
#include "pipeline/stages/OptimizeIRStage.h"
#include "preprocessor/FileSource.h"
#include "preprocessor/Preprocessor.h"
#include "syntax/RecursiveFunctionsGrammar.h"
#include "syntax/lr/LRParser.h"
#include "utils/Constants.h"

using Compilation::CompileTreeBuilder;
using Preprocessing::Preprocessor, Preprocessing::FileSource;

namespace Cli {
namespace fs = std::filesystem;

class Main {
  static Preprocessor prepare_preprocessor(const CompilerArguments& args) {
    Preprocessor preprocessor;

    const auto& includes = args.includes;
    const auto& main_filepath = args.main_path;

    preprocessor.add_source<FileSource>("main", main_filepath);
    preprocessor.set_main_source("main");

    for (auto& [name, path] : includes) {
      preprocessor.add_source<FileSource>(name, path);
    }

    return preprocessor;
  }

 public:
  static int main(int argc, char* argv[]) {
    return ExceptionsHandler::execute([argc, argv] {
      auto arguments = ArgumentsReader::read(argc, argv);

      Logger::set_level(arguments.verbosity_level);

      auto preprocessor = prepare_preprocessor(arguments);
      string program_text = preprocessor.process();

      if (arguments.emit_type == CompilerEmitType::PREPROCESSOR) {
        FilesystemManager::get_instance().save_to_file(
            arguments.output, StringPrinter{program_text, ".rec"});
        return;
      }

      auto tokens = Lexing::LexicalAnalyzer::get_tokens(program_text);

      auto [builders, _] = Syntax::get_recursive_functions_grammar();
      auto parser = Syntax::LRParser(Constants::grammar_filepath, builders);
      auto syntax_tree = parser.parse(tokens);

      if (!syntax_tree) {
        // TODO: refactor this
        throw std::runtime_error("Wrong syntax.");
      }

      CompileTreeBuilder compile_tree_builder;
      auto compile_tree = compile_tree_builder.build(**syntax_tree);

      // generate intermediate representation
      auto ir = IR::IRCompiler().get_ir(*compile_tree);

      if (arguments.emit_type == CompilerEmitType::IR) {
        FilesystemManager::get_instance().save_to_file(arguments.output,
                                                       IR::IRPrinter{ir});
        return;
      }

      auto optimized = OptimizeIRStage().apply(std::move(ir));
      if (arguments.emit_type == CompilerEmitType::ASSEMBLY) {
        FilesystemManager::get_instance().save_to_file(
            arguments.output, Assembly::AssemblyPrinter(optimized));
        return;
      }

      if (arguments.emit_type == CompilerEmitType::COMPILED) {
        auto saved_asm = FilesystemManager::get_instance().save_temporary(
            Assembly::AssemblyPrinter(optimized));

        // we must find std to link our program with it
        // for now we just search for ./std
        fs::path std_path = Constants::std_filepath;

        if (!fs::is_regular_file(std_path)) {
          throw std::runtime_error("Unable to find std library files.");
        }

        auto& output_path = arguments.output.replace_extension(".o");
        auto compile_command =
            fmt::format("g++ -o {} {} {}", output_path.string(),
                        saved_asm.path.string(), std_path.string());
        std::system(compile_command.c_str());
      }
    });
  }
};
}  // namespace Cli
