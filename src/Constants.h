#pragma once

namespace Constants {
// constexpr inline double infinity = 1e10;
//
// // isn't seven the most powerfully magical number
// constexpr inline size_t max_arguments = 7;

constexpr inline auto version = TLANG_VERSION;

constexpr inline auto constructor_name = "make";

constexpr inline auto entrypoint = "main";

constexpr inline auto std_library_relative_filepath = "lib/libstd.a";
constexpr inline auto std_include_relative_path = "include";

#ifdef NDEBUG
constexpr inline bool debug = false;
#else
constexpr inline bool debug = true;
#endif

}  // namespace Constants
