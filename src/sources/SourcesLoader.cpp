#include "SourcesLoader.h"

#include <fstream>
#include <sstream>

const SourcesLoader::FileSource& SourcesLoader::add_source(
    std::string import_name, std::filesystem::path path) {
  FileSource source;

  std::ifstream is(path);
  std::stringstream buffer;
  buffer << is.rdbuf();

  source.index = sources_.size();
  source.import_name = std::move(import_name);
  source.path = std::move(path);
  source.content = std::move(buffer.str());

  return sources_.emplace_back(std::move(source));
}
