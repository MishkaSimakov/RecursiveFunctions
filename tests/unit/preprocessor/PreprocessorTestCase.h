#pragma once

#include <fmt/std.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "RecursiveFunctions.h"

using Preprocessing::Preprocessor, Preprocessing::FileSource;
using std::string;
namespace fs = std::filesystem;

class PreprocessorTestCase : public ::testing::Test {
 protected:
  Preprocessor preprocessor;

  PreprocessorTestCase() { Logger::disable_category(Logger::ALL); }

  template <std::ranges::range T>
  static fs::path add_file(string filename, T&& contents) {
    fs::path temp_path = fs::temp_directory_path();

    auto path = temp_path / filename;
    path.replace_extension("rec");

    std::ofstream os(path);

    if (!os) {
      throw std::runtime_error(
          fmt::format("Failed to create file with name {} in directory {}",
                      filename, temp_path));
    }

    for (const auto& line : contents) {
      os << line << "\n";
    }

    return path;
  }

  template <std::ranges::range T>
  void add_main_source(T&& contents) {
    const auto name = "test";

    auto path = add_file(std::move(name), std::forward<T>(contents));
    preprocessor.add_source<FileSource>(name, path);
    preprocessor.set_main_source(name);
  }

  template <std::ranges::range T>
  string process_program(T&& program) {
    Preprocessor temp;

    auto path = add_file("main", std::forward<T>(program));
    temp.add_source<FileSource>("main", path);

    return temp.process();
  }

  void reset_preprocessor() { preprocessor = {}; }
};
