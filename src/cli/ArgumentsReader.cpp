#include "ArgumentsReader.h"

#include <fmt/format.h>

#include <argparse/argparse.hpp>
#include <filesystem>
#include <regex>

#include "Exceptions.h"

namespace fs = std::filesystem;

namespace Cli {

std::string ArgumentsReader::get_default_output_name(Front::EmitType type) {
  switch (type) {
    case Front::EmitType::AST:
    case Front::EmitType::IR:
      return "out.txt";
    case Front::EmitType::OBJECT:
      return "out.o";
    case Front::EmitType::EXECUTABLE:
      return "out";
  }

  unreachable("All output types should be enumerated above.");
}

void ArgumentsReader::parse_source_paths(
    const std::vector<std::string>& sources,
    Front::TeaFrontendConfiguration& config) {
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

      config.add_source(name, path);
    } else {
      // unnamed include
      // for this type of include name is a stem part of path
      // directory can be passed as parameter to this type of include
      // if so all files will be included recursively
      // for example file with relative path from given directory root
      // foo/baz/prog.tea will be available with `import "foo.baz.prog"`

      fs::path path = include;

      if (is_regular_file(path)) {
        fs::path path_copy = path;
        path_copy.replace_extension();

        auto include_name = std::regex_replace(std::string{path_copy},
                                               std::regex(separator), ".");

        config.add_source(include_name, path);

        continue;
      }

      if (!fs::is_directory(path)) {
        throw std::runtime_error(fmt::format(
            "Unnamed include \"{}\" must be regular file or directory",
            path.string()));
      }

      for (auto subfile : fs::recursive_directory_iterator(path)) {
        if (!subfile.is_regular_file()) {
          continue;
        }

        if (subfile.path().extension() != ".tea" &&
            subfile.path().extension() != ".team") {
          continue;
        }

        std::string relative_path = relative(subfile, path).replace_extension();
        auto include_name =
            std::regex_replace(relative_path, std::regex(separator), ".");

        config.add_source(include_name, subfile.path());
      }
    }
  }
}

std::filesystem::path ArgumentsReader::parse_output(std::string output,
                                                    Front::EmitType emit_type) {
  // empty output means that default value is used
  if (output.empty()) {
    switch (emit_type) {
      case Front::EmitType::AST:
      case Front::EmitType::IR:
        // write to stdout
        return {};
      case Front::EmitType::OBJECT:
      case Front::EmitType::EXECUTABLE:
        output = get_default_output_name(emit_type);
        break;
    }
  }

  fs::path output_path = output;

  // output must be directory or path
  if (fs::is_directory(output)) {
    output_path /= get_default_output_name(emit_type);
  }

  return fs::absolute(output_path).lexically_normal();
}

Front::EmitType ArgumentsReader::get_emit_type(std::string_view name) {
  if (name == "ir") {
    return Front::EmitType::IR;
  }
  if (name == "ast") {
    return Front::EmitType::AST;
  }
  if (name == "obj") {
    return Front::EmitType::OBJECT;
  }
  if (name == "exe") {
    return Front::EmitType::EXECUTABLE;
  }
  throw std::runtime_error("unknown compiler emit type.");
}

Front::TeaFrontendConfiguration ArgumentsReader::read(int argc, char* argv[]) {
  argparse::ArgumentParser parser("compiler");
  parser.add_description("Compiler for TeaLang ☕️");

  parser.add_argument("sources")
      .nargs(argparse::nargs_pattern::at_least_one)
      .help(
          "Files to compile. You can write <name>:<filepath> to create "
          "implicitly named include, <filepath> to deduce include name "
          "automatically or <directory path> to include all files in "
          "directory recursively.");

  parser.add_argument("-o", "--output").default_value("").help("output file");

  parser.add_argument("--emit")
      .choices("ir", "ast", "obj", "exe")
      .default_value("exe")
      .help("compiler output type: ir, ast, obj, exe");

  try {
    parser.parse_args(argc, argv);
  } catch (const std::exception& err) {
    throw ArgumentsParseException(err.what());
  }

  Front::TeaFrontendConfiguration result;

  parse_source_paths(parser.get<std::vector<std::string>>("sources"), result);
  result.emit_type = get_emit_type(parser.get<std::string>("emit"));
  result.output_file = parse_output(parser.get("output"), result.emit_type);

  return result;
}

}  // namespace Cli
