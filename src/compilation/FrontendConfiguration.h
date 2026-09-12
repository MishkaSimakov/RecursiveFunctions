#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include "errors/Helpers.h"

namespace Front {

enum class EmitType { AST, IR, OBJECT, EXECUTABLE, MODULES_LIST };

struct TeaFrontendConfiguration {
  std::unordered_map<std::string, std::filesystem::path> sources;
  std::filesystem::path output_file;
  EmitType emit_type;

  void add_source(std::string name, std::filesystem::path path);
};

inline std::string to_string(EmitType emit_type) {
  switch (emit_type) {
    case EmitType::AST:
      return "AST";
    case EmitType::IR:
      return "IR";
    case EmitType::OBJECT:
      return "OBJ";
    case EmitType::EXECUTABLE:
      return "EXE";
    case EmitType::MODULES_LIST:
      return "MODULES_LIST";
  }

  unreachable("All emit types should be enumerated above.");
}

}  // namespace Front
