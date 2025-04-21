#pragma once

#include <filesystem>
#include <utility>

namespace Constants {
// constexpr inline double infinity = 1e10;
//
// // isn't seven the most powerfully magical number
// constexpr inline size_t max_arguments = 7;

constexpr inline auto constructor_name = "make";

constexpr inline auto entrypoint = "main";
extern const bool is_installed_build;

constexpr inline auto lexis_relative_filepath = "lexis/lexis.lx";
constexpr inline auto grammar_relative_filepath = "grammar/grammar.lr";

inline std::filesystem::path GetRuntimeFilePath(
    std::filesystem::path relative_path) {
  {
    if (is_installed_build) {
      // TODO: get this path from OS
      return std::filesystem::path("/usr/local/share/tlang/files") /
             relative_path;
    } else {
      return std::filesystem::path(FILES_DIRECTORY) / relative_path;
    }
  }
}

inline std::filesystem::path GetBuildFilePath(
    std::filesystem::path relative_path) {
  return std::filesystem::path(FILES_DIRECTORY) / relative_path;
}

#ifdef NDEBUG
constexpr inline bool debug = false;
#else
constexpr inline bool debug = true;
#endif

}  // namespace Constants
