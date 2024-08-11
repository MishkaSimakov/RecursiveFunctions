#pragma once
#include "intermediate_representation/Program.h"

namespace Passes {
class Analyser {
 private:
  bool is_valid = false;

 protected:
  virtual void perform_analysis(const IR::Program&) = 0;

 public:
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
