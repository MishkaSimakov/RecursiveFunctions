#pragma once

#include <utility>

namespace Constants {
constexpr inline double infinity = 1e10;

// isn't seven the most powerfully magical number
constexpr inline size_t max_arguments = 7;

constexpr inline auto entrypoint = "main";
constexpr inline auto grammar_filepath = "files/grammar/grammar.lr";
constexpr inline auto lexis_filepath = "files/lexis/lexis.lx";
constexpr inline auto std_filepath = "files/std/reclib.asm";
}  // namespace Constants
