#pragma once

#include <exception>

namespace Syntax {
// TODO: store location in code where this token was met

struct UnexpectedTokenException : std::exception {
  std::string message;

  explicit UnexpectedTokenException(TokensBitset expected)
      : message("Unexpected token met.") {}

  const char* what() const noexcept override { return message.c_str(); }
};
}