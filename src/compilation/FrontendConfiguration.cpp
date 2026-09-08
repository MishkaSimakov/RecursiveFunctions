#include "FrontendConfiguration.h"

#include <fmt/format.h>

namespace Front {

void TeaFrontendConfiguration::add_source(std::string name,
                                          std::filesystem::path path) {
  auto [_, inserted] = sources.emplace(name, path);

  if (!inserted) {
    throw std::runtime_error(fmt::format(
        "File {} was already included with different name.", path.c_str()));
  }
}

}  // namespace Front
