#pragma once

#include <filesystem>

namespace Syntax {
class GrammarGenerator {
 public:
  static void generate_grammar(const std::filesystem::path& input_path,
                               const std::filesystem::path& table_path,
                               const std::filesystem::path& builders_path);
};
}  // namespace Syntax
