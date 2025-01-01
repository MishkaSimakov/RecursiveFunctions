#pragma once

#include <argparse/argparse.hpp>

#include "ArgumentsReader.h"
#include "compilation/TeaFrontend.h"

#include "errors/ExceptionsHandler.h"

#include "log/Logger.h"


namespace Cli {
namespace fs = std::filesystem;

class Main {
 public:
  static int main(int argc, char* argv[]) {
    return ExceptionsHandler::execute([argc, argv] {
      auto arguments = ArgumentsReader::read(argc, argv);

      Logger::set_level(arguments.verbosity_level);

      auto result = TeaFrontend().compile(arguments.sources);
      // result must be a bunch of IR files
      // compile each module into IR

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
