#pragma once
#include "intermediate_representation/Program.h"

namespace Analysis {
class Analyser {
 public:
  bool is_valid = false;

  virtual void apply(const IR::Program&) = 0;

  virtual ~Analyser() = default;
};
}  // namespace Analysis
