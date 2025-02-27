#pragma once

#include <array>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace Cli {
// import name / filepath
using SourcesList = std::unordered_map<std::string, std::filesystem::path>;

struct CompilerArguments {
  SourcesList sources;
  std::filesystem::path output;
  bool dump_ast;
};

class ArgumentsReader {
  constexpr static auto kDefaultOutputName = "out";
  constexpr static auto kSourceNamePathDelimiter = ":";

  static SourcesList parse_source_paths(
      std::vector<std::string> sources);

  static std::filesystem::path parse_output(const std::string& output);
 public:
  static CompilerArguments read(int argc, char* argv[]);
};
}  // namespace Cli
