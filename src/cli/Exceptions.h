#pragma once

#include <exception>
#include <string>

namespace Cli {
struct ArgumentsParseException : std::exception {
  std::string message;

  ArgumentsParseException(std::string message) : message(std::move(message)) {}

  const char* what() const noexcept override { return message.c_str(); }
};
}  // namespace Cli