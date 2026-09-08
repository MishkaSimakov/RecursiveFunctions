#pragma once

#include <array>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "compilation/FrontendConfiguration.h"

namespace Cli {

class ArgumentsReader {
  constexpr static auto kSourceNamePathDelimiter = ":";

  static std::string get_default_output_name(Front::EmitType type);

  static void parse_source_paths(const std::vector<std::string>& sources,
                                 Front::TeaFrontendConfiguration& config);

  static std::filesystem::path parse_output(std::string output,
                                            Front::EmitType emit_type);

  static Front::EmitType get_emit_type(std::string_view name);

 public:
  static Front::TeaFrontendConfiguration read(int argc, char* argv[]);
};

}  // namespace Cli
