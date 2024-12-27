#include "ArgumentsReader.h"

#include <fmt/format.h>

#include <argparse/argparse.hpp>
#include <filesystem>
#include <regex>

#include "Exceptions.h"

namespace fs = std::filesystem;

Cli::SourcesList Cli::ArgumentsReader::parse_source_paths(
    std::vector<std::string> sources) {
  SourcesList result;
  auto add_source = [&result](const std::string& name, const fs::path& path) {
    auto [_, was_emplaced] = result.emplace(name, path);
    if (!was_emplaced) {
      throw std::runtime_error(fmt::format(
          "File {} was already included with different name.", path.c_str()));
    }
  };

  std::string separator{fs::path::preferred_separator};

  for (auto& include : sources) {
    if (include.contains(kSourceNamePathDelimiter)) {
      // named include
      size_t index = include.find(kSourceNamePathDelimiter);
      std::string name = include.substr(0, index);

      if (name.empty()) {
        throw std::runtime_error("Include name must be non-empty.");
      }

      fs::path path = include.substr(index + 1, include.size() - index);

      if (!fs::is_regular_file(path)) {
        throw std::runtime_error(fmt::format(
            "Named include \"{}\" must refer to regular file.", name));
      }

      add_source(name, path);
    } else {
      // unnamed include
      // for this type of include name is stem part of path
      // directory can be passed as parameter to this type of include
      // if so all files will be included recursively
      // for example file with relative path from given directory root
      // foo/baz/prog.rec will be available with #include "foo.baz.prog"

      fs::path path = include;

      if (is_regular_file(path)) {
        fs::path path_copy = path;
        path_copy.replace_extension();

        auto include_name = std::regex_replace(std::string{path_copy},
                                               std::regex(separator), ".");

        add_source(include_name, path);

        continue;
      }

      if (!fs::is_directory(path)) {
        throw std::runtime_error(fmt::format(
            "Unnamed include \"{}\" must be regular file or directory",
            path.string()));
      }

      for (auto subfile : fs::recursive_directory_iterator(path)) {
        if (subfile.is_regular_file()) {
          std::string relative_path =
              relative(subfile, path).replace_extension();
          auto include_name =
              std::regex_replace(relative_path, std::regex(separator), ".");

          add_source(include_name, subfile.path());
        }
      }
    }
  }

  return result;
}

std::filesystem::path Cli::ArgumentsReader::parse_output(
    const std::string& output) {
  fs::path output_path = output;

  // output must be directory or path
  if (fs::is_directory(output)) {
    output_path /= kDefaultOutputName;
  }

  return output_path;
}

Cli::CompilerArguments Cli::ArgumentsReader::read(int argc, char* argv[]) {
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
      .default_value(std::filesystem::current_path().string())
      .help("specify output file name");

  parser.add_argument("-d", "--debug")
      .default_value(false)
      .implicit_value(true)
      .help("turn on debug mode");

  try {
    parser.parse_args(argc, argv);
  } catch (const std::exception& err) {
    throw ArgumentsParseException(err.what());
  }

  // fill arguments object
  CompilerArguments arguments;
  arguments.sources =
      parse_source_paths(parser.get<std::vector<std::string>>("include"));
  arguments.output = parse_output(parser.get("output"));

  arguments.debug = parser.get<bool>("debug");
  arguments.verbosity_level = verbosity;
  arguments.emit_type = CompilerEmitType::COMPILED;

  return arguments;
}
