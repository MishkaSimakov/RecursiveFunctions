#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

namespace Front {

enum class EmitType { AST, IR };

struct TeaFrontendConfiguration {
  std::unordered_map<std::string, std::filesystem::path> sources;
  std::filesystem::path output_file;
  EmitType emit_type;
};

}  // namespace Front
