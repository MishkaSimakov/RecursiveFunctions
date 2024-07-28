#pragma once

namespace Passes {
class PassManager;

class Pass {
 protected:
  PassManager& manager_;

 public:
  explicit Pass(PassManager& manager) : manager_(manager) {}

  virtual void apply() = 0;

  virtual ~Pass() = default;
};
}  // namespace Passes
