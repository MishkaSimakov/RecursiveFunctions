#pragma once

#include <fmt/core.h>

#include <exception>
#include <filesystem>
#include <string>

namespace Preprocessing {
namespace fs = std::filesystem;

struct MainSourceNotFoundException : std::exception {
  std::string message;

  explicit MainSourceNotFoundException(const std::string& source_name)
      : message(fmt::format("File source located in {:?} was not found.",
                            source_name)) {}

  const char* what() const noexcept override { return message.c_str(); }
};

struct FileSourceNotFoundException : std::exception {
  std::string message;

  explicit FileSourceNotFoundException(const fs::path& filepath)
      : message(fmt::format("File source located in {:?} was not found.",
                            filepath.string())) {}

  const char* what() const noexcept override { return message.c_str(); }
};

struct IncludeSourceNotFoundException : std::exception {
  std::string message;

  explicit IncludeSourceNotFoundException(const std::string& in_source,
                                          const std::string& include_name)
      : message(fmt::format("In source {:?} include {:?} was not found.",
                            in_source, include_name)) {}

  const char* what() const noexcept override { return message.c_str(); }
};
}  // namespace Preprocessing
