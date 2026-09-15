#include "FrontendConfiguration.h"

#include <fmt/format.h>

namespace Front {

void TeaFrontendConfiguration::add_source(std::string name,
                                          SourceConfig config) {
  auto [itr, inserted] = sources.emplace(std::move(name), config);

  if (!inserted) {
    if (!itr->second.is_std && !config.is_std) {
      throw std::runtime_error(
          fmt::format("Two sources must not share the same name. {:?} and {:?} "
                      "are both named {:?}.",
                      itr->second.path.string(), config.path.string(), name));
    }

    if (!itr->second.is_std || !config.is_std) {
      const auto& user_file_path =
          config.is_std ? itr->second.path : config.path;
      throw std::runtime_error(
          fmt::format("The source file {:?} named {:?} conflicts with a "
                      "standard library source.",
                      user_file_path.string(), name));
    }

    unreachable("Two std sources can't share the same name.");
  }
}

}  // namespace Front
