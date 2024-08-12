#pragma once
#include "intermediate_representation/Program.h"
#include "passes/PassManager.h"

namespace Passes {
class Analyser {
 private:
  bool is_valid = false;
  PassManager& pass_manager_;

 protected:
  virtual void perform_analysis(const IR::Program&) = 0;

  template <typename T>
  T& get_analysis() {
    return pass_manager_.get_analysis<T>();
  }

 public:
  explicit Analyser(PassManager& pass_manager) : pass_manager_(pass_manager) {}

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
