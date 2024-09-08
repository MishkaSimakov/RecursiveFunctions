#include "PreprocessStage.h"

#include <filesystem>

#include "preprocessor/FileSource.h"
#include "preprocessor/Preprocessor.h"

namespace fs = std::filesystem;

std::string PreprocessStage::apply(CompilerSettings settings) {
  Preprocessing::Preprocessor preprocessor;

  const auto& includes = settings.includes;
  const auto& main_filepath = settings.main_path;

  if (!fs::is_regular_file(main_filepath)) {
    throw std::runtime_error(fmt::format("No such file {}", main_filepath));
  }

  preprocessor.add_source<Preprocessing::FileSource>("main", main_filepath);
  preprocessor.set_main_source("main");

  string separator{fs::path::preferred_separator};

  for (auto& include : includes) {
    if (include.contains(kIncludeNamePathDelimiter)) {
      // named include
      size_t index = include.find(kIncludeNamePathDelimiter);
      string name = include.substr(0, index);

      if (name.empty()) {
        throw std::runtime_error("Include name must be non-empty.");
      }

      fs::path path = include.substr(index + 1, include.size() - index);

      if (is_directory(path)) {
        throw std::runtime_error("Directory import can not be named.");
      }

      preprocessor.add_source<Preprocessing::FileSource>(name, std::move(path));
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

        auto include_name =
            std::regex_replace(string{path_copy}, std::regex(separator), ".");

        preprocessor.add_source<Preprocessing::FileSource>(include_name, path);
        continue;
      }

      for (auto subfile : fs::recursive_directory_iterator(path)) {
        if (subfile.is_regular_file()) {
          string relative_path = relative(subfile, path).replace_extension();
          auto include_name =
              std::regex_replace(relative_path, std::regex(separator), ".");

          preprocessor.add_source<Preprocessing::FileSource>(include_name,
                                                             subfile.path());
        }
      }
    }
  }

  return preprocessor.process();
}
