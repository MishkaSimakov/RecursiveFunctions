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
#include "log/Logger.h"
#include "passes/PassManager.h"
#include "pipeline/stages/OptimizeIRStage.h"
#include "sources/SourcesLoader.h"
#include "syntax/RecursiveFunctionsGrammar.h"
#include "syntax/lr/LRParser.h"
#include "utils/Constants.h"

using Compilation::CompileTreeBuilder;

namespace Cli {
namespace fs = std::filesystem;

class Main {
 public:
  static int main(int argc, char* argv[]) {
    return ExceptionsHandler::execute([argc, argv] {
      auto arguments = ArgumentsReader::read(argc, argv);

      Logger::set_level(arguments.verbosity_level);

      SourcesLoader sources_loader;

      // process each file separately
      for (auto& [name, path] : arguments.sources) {
        const auto& source = sources_loader.add_source(name, path);
        auto tokens = Lexis::LexicalAnalyzer::get_tokens(source);

        auto [builders, _] = Syntax::get_recursive_functions_grammar();
        auto parser = Syntax::LRParser(Constants::grammar_filepath, builders);
        auto syntax_tree = parser.parse(tokens);

        CompileTreeBuilder compile_tree_builder;
        auto compile_tree = compile_tree_builder.build(*syntax_tree);

        // somewhere here we should check that all imports are correct

        auto ir = IR::IRCompiler().get_ir(*compile_tree);
      }

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
