#pragma once

#include <vector>

#include "intermediate_representation/Value.h"

namespace Passes {
class Disentangler {
 public:
  std::vector<std::pair<IR::Value, IR::Value>> disentangle(
      const std::vector<std::pair<IR::Value, IR::Value>>&, IR::Value);
};
}  // namespace Passes
