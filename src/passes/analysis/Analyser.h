#pragma once
#include "intermediate_representation/Program.h"

namespace Passes {
class PassManager;

class Analyser {
  bool is_valid = false;

 protected:
  PassManager& manager_;

  virtual void perform_analysis(const IR::Program&) = 0;

 public:
  explicit Analyser(PassManager& manager) : manager_(manager) {}

  void analyse(const IR::Program& program) {
    if (is_valid) {
      return;
    }

    perform_analysis(program);
    is_valid = true;
  }

  void invalidate() { is_valid = false; }

  virtual ~Analyser() = default;
};
}  // namespace Passes
