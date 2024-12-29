#pragma once

#include <argparse/argparse.hpp>

#include "ArgumentsReader.h"
#include "assembly/AssemblyPrinter.h"
#include "ast/ASTPrinter.h"
#include "errors/ExceptionsHandler.h"
#include "lexis/LexicalAnalyzer.h"
#include "log/Logger.h"
#include "passes/PassManager.h"
#include "sources/SourceManager.h"
#include "syntax/lr/LRParser.h"
#include "utils/Constants.h"

namespace Cli {
namespace fs = std::filesystem;

class Main {
 public:
  static int main(int argc, char* argv[]) {
    return ExceptionsHandler::execute([argc, argv] {
      auto arguments = ArgumentsReader::read(argc, argv);

      Logger::set_level(arguments.verbosity_level);

      SourceManager source_manager;

      // setup parser and lexical analyzer, load tables
      Lexis::LexicalAnalyzer lexical_analyzer(Constants::lexis_filepath,
                                              source_manager);
      auto parser =
          Syntax::LRParser(Constants::grammar_filepath, source_manager);

      // process each file separately
      for (auto& [name, path] : arguments.sources) {
        SourceLocation begin = source_manager.load(path);
        lexical_analyzer.set_location(begin);
        auto ast = parser.parse(lexical_analyzer);

        ASTPrinter(ast, std::cout, source_manager).traverse();

        // CompileTreeBuilder compile_tree_builder;
        // auto compile_tree = compile_tree_builder.build(*syntax_tree);

        // auto ir = IR::IRCompiler().get_ir(*compile_tree);
      }

      //   if (arguments.emit_type == CompilerEmitType::IR) {
      //     FilesystemManager::get_instance().save_to_file(arguments.output,
      //                                                    IR::IRPrinter{ir});
      //     return;
      //   }
      //
      //   auto optimized = OptimizeIRStage().apply(std::move(ir));
      //   if (arguments.emit_type == CompilerEmitType::ASSEMBLY) {
      //     FilesystemManager::get_instance().save_to_file(
      //         arguments.output, Assembly::AssemblyPrinter(optimized));
      //     return;
      //   }
      //
      //   if (arguments.emit_type == CompilerEmitType::COMPILED) {
      //     auto saved_asm = FilesystemManager::get_instance().save_temporary(
      //         Assembly::AssemblyPrinter(optimized));
      //
      //     // we must find std to link our program with it
      //     // for now we just search for ./std
      //     fs::path std_path = Constants::std_filepath;
      //
      //     if (!fs::is_regular_file(std_path)) {
      //       throw std::runtime_error("Unable to find std library files.");
      //     }
      //
      //     auto& output_path = arguments.output.replace_extension(".o");
      //     auto compile_command =
      //         fmt::format("g++ -o {} {} {}", output_path.string(),
      //                     saved_asm.path.string(), std_path.string());
      //     std::system(compile_command.c_str());
      //   }
    });
  }
};
}  // namespace Cli
