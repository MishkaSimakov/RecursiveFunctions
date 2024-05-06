#pragma once

#include <fmt/core.h>

#include <string>

namespace Lexing {
using std::string;

struct UnexpectedSymbolException : std::exception {
  constexpr static size_t kPrintRadius = 20;

  string message;

  static auto get_symbol_vicinity(const string& program, size_t position) {
    size_t begin = position >= kPrintRadius ? position - kPrintRadius : 0;
    size_t end = std::min(program.size(), begin + kPrintRadius * 2 + 1);

    return std::pair{program.substr(begin, kPrintRadius * 2 + 1),
                     end - position};
  }

  explicit UnexpectedSymbolException(const string& program, size_t position)
      : message([&program, position] {
          auto [vicinity, offset] = get_symbol_vicinity(program, position);
          auto message =
              fmt::format("Unexpected symbol {:?} occured in program: {}",
                          program[position], vicinity);

          message += "\n" + string(message.size() - offset, ' ') + "↑\n";

          return message;
        }()) {}

  const char* what() const noexcept override { return message.c_str(); }
};
}  // namespace Lexing
