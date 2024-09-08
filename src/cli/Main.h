#pragma once

#include <argparse/argparse.hpp>

#include "ArgumentsReader.h"
#include "ExceptionsHandler.h"
#include "RecursiveFunctions.h"
#include "assembly/AssemblyPrinter.h"
#include "filesystem/FilesystemManager.h"
#include "intermediate_representation/IRPrinter.h"
#include "intermediate_representation/compiler/IRCompiler.h"
#include "passes/PassManager.h"
#include "pipeline/stages/OptimizeIRStage.h"

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

      auto tokens = LexicalAnalyzer::get_tokens(program_text);

      auto syntax_tree = SyntaxTreeBuilder::build(
          tokens, RecursiveFunctionsSyntax::GetSyntax(),
          RecursiveFunctionsSyntax::RuleIdentifiers::PROGRAM);

      CompileTreeBuilder compile_tree_builder;
      auto compile_tree = compile_tree_builder.build(*syntax_tree);

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

        auto& output_path = arguments.output.replace_extension(".o");
        auto compile_command = fmt::format(
            "g++ {} -o {}", saved_asm.path.string(), output_path.string());
        std::system(compile_command.c_str());
      }
    });
  }
};
}  // namespace Cli
