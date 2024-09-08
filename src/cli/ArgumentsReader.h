#pragma once

#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace Cli {
enum class CompilerEmitType { PREPROCESSOR, IR, ASSEMBLY, COMPILED };

struct CompilerArguments {
  using IncludesStorage =
      std::vector<std::pair<std::string, std::filesystem::path>>;

  IncludesStorage includes;
  std::filesystem::path main_path;

  size_t verbosity_level;
  bool debug;
  std::filesystem::path output;
  CompilerEmitType emit_type;
};

class ArgumentsReader {
  constexpr static auto kDefaultOutputName = "out";
  constexpr static auto kIncludeNamePathDelimiter = ":";

  static CompilerArguments::IncludesStorage parse_includes(
      std::vector<std::string> includes);

  static std::filesystem::path parse_output(std::string output);
 public:
  static CompilerArguments read(int argc, char* argv[]);
};
}  // namespace Cli
