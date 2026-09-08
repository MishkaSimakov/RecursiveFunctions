#include <fmt/base.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include <argparse/argparse.hpp>

#include "ArgumentsReader.h"
#include "Constants.h"
#include "compilation/TeaFrontend.h"
#include "errors/ExceptionsHandler.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"
#include "utils/FileDescriptor.h"

const bool Constants::is_installed_build = BUILD_FOR_INSTALLATION;

namespace Cli {
namespace fs = std::filesystem;

class Main {
  static FileDescriptor get_output_fd(const std::filesystem::path& output_path,
                                      Front::EmitType emit_type) {
    const bool prohibit_stdout =
        emit_type != Front::EmitType::AST && emit_type != Front::EmitType::IR;
    const bool is_executable = emit_type == Front::EmitType::EXECUTABLE;

    if (output_path.empty()) {
      if (prohibit_stdout) {
        throw std::runtime_error(
            fmt::format("Can't emit {} to stdout. Specify output file path.",
                        to_string(emit_type)));
      }

      int stdout_fd = fileno(stdout);
      if (stdout_fd == -1) {
        throw std::runtime_error("Failed to open stdout.");
      }

      int fd = dup(stdout_fd);
      if (fd == -1) {
        throw std::runtime_error("Failed to duplicate stdout fd.");
      }

      return FileDescriptor(fd);
    }

    mode_t mode = S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR;
    if (is_executable) {
      mode |= S_IXGRP | S_IXUSR;
    }

    int fd = open(output_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);

    if (fd == -1) {
      throw std::runtime_error(
          fmt::format("Failed to open output file: {}.", strerror(errno)));
    }

    return FileDescriptor(fd);
  }

  static void add_std_includes(Front::TeaFrontendConfiguration& config) {
    const char* std_filenames[] = {"io"};

    for (const char* name : std_filenames) {
      auto path = Constants::GetRuntimeFilePath(
          fs::path(Constants::std_include_relative_path) / name);
      path.replace_extension(".tea");

      auto [_, inserted] = config.sources.emplace(name, path);

      if (!inserted) {
        throw std::runtime_error("Duplicate module name.");
      }
    }
  }

  static void emit_ir(std::unique_ptr<llvm::Module> module,
                      const FileDescriptor& fd) {
    llvm::raw_fd_ostream llvm_out(fd.get(), false);
    module->print(llvm_out, nullptr);
  }

  static void emit_object(std::unique_ptr<llvm::Module> module,
                          const FileDescriptor& fd) {
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

    llvm::raw_fd_ostream dest(fd.get(), false);
    llvm::legacy::PassManager pass;

    if (target_machine->addPassesToEmitFile(
            pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
      throw std::runtime_error(
          "Target machine doesn't support object output format.");
    }

    pass.run(*module);
    dest.flush();
  }

  static void emit_executable(std::unique_ptr<llvm::Module> module,
                              const FileDescriptor& fd) {
    const auto tmp_fd = FileDescriptor::make_temp();

    // write object file
    emit_object(std::move(module), tmp_fd);

    // link with std
    const auto std_path =
        Constants::GetRuntimeFilePath(Constants::std_library_relative_filepath);
    const auto link_command =
        fmt::format("clang++ /dev/fd/{} {} -o /dev/fd/{}", std_path.string(),
                    tmp_fd.get(), fd.get());

    int link_status = system(link_command.c_str());
    if (link_status != 0) {
      throw std::runtime_error("Failed to link with clang++.");
    }
  }

 public:
  static int main(int argc, char* argv[]) {
    return ExceptionsHandler::execute([argc, argv] {
      auto config = ArgumentsReader::read(argc, argv);

      add_std_includes(config);

      auto front = Front::TeaFrontend(config);

      auto llvm_module = front.compile();

      if (config.emit_type == Front::EmitType::AST) {
        return;
      }

      assert(llvm_module != nullptr);

      const auto fd = get_output_fd(config.output_file, config.emit_type);

      switch (config.emit_type) {
        case Front::EmitType::AST:
          assert(false && "Should've been handled above");
          break;
        case Front::EmitType::IR:
          emit_ir(std::move(llvm_module), fd);
          break;
        case Front::EmitType::OBJECT:
          emit_object(std::move(llvm_module), fd);
          break;
        case Front::EmitType::EXECUTABLE:
          emit_executable(std::move(llvm_module), fd);
          break;
      }
    });
  }
};
}  // namespace Cli

int main(int argc, char* argv[]) { return Cli::Main::main(argc, argv); }
