#pragma once

#include <exception>
#include <filesystem>

namespace Syntax {
namespace fs = std::filesystem;

struct NoRulesMatchedException : std::exception {
  string message;

  explicit NoRulesMatchedException()
      : message("No rules matched your program.") {}

  const char* what() const noexcept override { return message.c_str(); }
};
}  // namespace Syntax
