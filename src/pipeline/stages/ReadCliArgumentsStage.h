#pragma once
#include <span>
#include <vector>
#include <string>

struct CompilerSettings {
  std::vector<std::string> includes;
  std::string main_path;
  size_t verbosity;
  bool debug;
  std::string output_path;
};

class ReadCliArgumentsStage {
 public:
  using input = std::span<char*>;
  using output = CompilerSettings;
  constexpr static auto name = "read_cli_arguments";

  CompilerSettings apply(std::span<char*> args);
};
