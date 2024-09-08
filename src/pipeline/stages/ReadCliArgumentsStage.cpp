#include "ReadCliArgumentsStage.h"

#include <cli/Exceptions.h>

#include <argparse/argparse.hpp>

CompilerSettings ReadCliArgumentsStage::apply(std::span<char*> args) {
  argparse::ArgumentParser parser("interpeter");
  parser.add_description(
      "Compiler for general recursive functions "
      "(https://en.wikipedia.org/wiki/General_recursive_function).");

  parser.add_argument("filepath").help("Path to main file.");
  parser.add_argument("-i", "--include")
      .nargs(argparse::nargs_pattern::any)
      .help(
          "adds include paths. You can write <name>:<filepath> to create "
          "implicitly named include, <filepath> to deduce include name "
          "automatically or <directory path> to include all files in "
          "directory recursively.");

  size_t verbosity = 0;
  parser.add_argument("-v")
      .action([&](const auto&) { ++verbosity; })
      .append()
      .default_value(false)
      .implicit_value(true)
      .nargs(0)
      .help(
          "increase program verbosity. -v - show only warnings, -vv - show "
          "info and "
          "warnings, -vvv - show all log messages.");

  parser.add_argument("-o", "--output")
      .default_value("")
      .help("specify output file name");

  parser.add_argument("-d", "--debug")
      .default_value(false)
      .implicit_value(true)
      .help("turn on debug mode");

  try {
    parser.parse_args(args.size(), args.data());
  } catch (const std::exception& err) {
    throw Cli::ArgumentsParseException(err.what());
  }

  // fill settings
  CompilerSettings settings;

  settings.includes = parser.get<std::vector<std::string>>("include");
  settings.main_path = parser.get("filepath");
  settings.verbosity = verbosity;
  settings.debug = parser.is_used("debug");
  settings.output_path = parser.get("output");

  return settings;
}
