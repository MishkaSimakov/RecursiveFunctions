#pragma once

#include <array>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "compilation/FrontendConfiguration.h"

namespace Cli {

class ArgumentsReader {
  constexpr static auto kDefaultOutputName = "out";
  constexpr static auto kSourceNamePathDelimiter = ":";

  static void parse_source_paths(const std::vector<std::string>& sources,
                                 Front::TeaFrontendConfiguration& config);

  static std::filesystem::path parse_output(const std::string& output);

  static Front::EmitType get_emit_type(std::string_view name);

 public:
  static Front::TeaFrontendConfiguration read(int argc, char* argv[]);
};

}  // namespace Cli
