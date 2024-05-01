#pragma once

#include <fmt/core.h>

namespace Lexing {
struct UnexpectedSymbolException : std::exception {
  string message;

  static string get_symbol_vicinity(const string& program, size_t position) {
    size_t radius = 20;
    size_t begin = position >= radius ? position - radius : 0;

    auto left_vicinity = program.substr(begin, position - begin);
    auto right_vicinity = program.substr(position + 1, radius);

    return left_vicinity + " >>>" + program[position] + "<<< " + right_vicinity;
  }

  explicit UnexpectedSymbolException(const string& program, size_t position)
      : message(fmt::format("Unexpected symbol {:?} occured in program: {:?}.",
                            program[position],
                            get_symbol_vicinity(program, position))) {}

  const char* what() const noexcept override { return message.c_str(); }
};
}  // namespace Lexing
