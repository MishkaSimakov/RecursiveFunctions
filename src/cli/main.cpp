#include <fmt/base.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

#include <argparse/argparse.hpp>
#include <fstream>

#include "ArgumentsReader.h"
#include "Constants.h"
#include "compilation/TeaFrontend.h"
#include "errors/ExceptionsHandler.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"

const bool Constants::is_installed_build = BUILD_FOR_INSTALLATION;

namespace Cli {
namespace fs = std::filesystem;

class Main {
  static void emit_ir(std::unique_ptr<llvm::Module> module,
                      const Front::TeaFrontendConfiguration& config) {
    std::ofstream ofs;

    std::ostream& out = [&]() -> std::ostream& {
      if (config.output_file.empty()) {
        return std::cout;
      }

      ofs.open(config.output_file);
      return ofs;
    }();

    llvm::raw_os_ostream llvm_out(out);
    module->print(llvm_out, nullptr);
  }

  static void emit_binary(std::unique_ptr<llvm::Module> module,
                          const Front::TeaFrontendConfiguration& config) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();

    auto target_triple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(llvm::Triple(target_triple));

    std::string error;
    auto target =
        llvm::TargetRegistry::lookupTarget(module->getTargetTriple(), error);

    if (!target) {
      throw std::runtime_error(
          fmt::format("Error while obtaining target machine info: {}", error));
    }

    auto CPU = "generic";
    auto features = "";

    llvm::TargetOptions opt;
    auto target_machine = target->createTargetMachine(
        llvm::Triple(target_triple), CPU, features, opt, std::nullopt);

    if (target_machine == nullptr) {
      throw std::runtime_error("Failed to load target machine info.");
    }

    module->setDataLayout(target_machine->createDataLayout());

    std::error_code error_code;
    llvm::raw_fd_ostream dest(config.output_file.c_str(), error_code,
                              llvm::sys::fs::OF_None);

    if (error_code) {
      throw std::runtime_error(
          fmt::format("Could not open file: {}.", error_code.message()));
    }

    llvm::legacy::PassManager pass;
    auto file_type = llvm::CodeGenFileType::ObjectFile;

    if (target_machine->addPassesToEmitFile(pass, dest, nullptr, file_type)) {
      throw std::runtime_error(
          "Target machine doesn't support binary output format.");
    }

    pass.run(*module);
    dest.flush();
  }

 public:
  static int main(int argc, char* argv[]) {
    return ExceptionsHandler::execute([argc, argv] {
      auto config = ArgumentsReader::read(argc, argv);

      auto front = Front::TeaFrontend(config);
      auto llvm_module = front.compile();

      if (config.emit_type == Front::EmitType::AST) {
        return;
      }

      assert(llvm_module != nullptr);

      if (config.emit_type == Front::EmitType::IR) {
        emit_ir(std::move(llvm_module), config);
      } else if (config.emit_type == Front::EmitType::BINARY) {
        emit_binary(std::move(llvm_module), config);
      }
    });
  }
};
}  // namespace Cli

int main(int argc, char* argv[]) { return Cli::Main::main(argc, argv); }
