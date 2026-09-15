#pragma once
#include "compilation/FrontendConfiguration.h"

namespace Cli {

struct Config : Front::TeaFrontendConfiguration {
  // Path to the directory containing tlang std library and headers.
  // If empty, then the default path would be used.
  std::string resource_dir;
};

}  // namespace Cli
