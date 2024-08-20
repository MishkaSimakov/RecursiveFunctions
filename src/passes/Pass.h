#pragma once

namespace Passes {
class PassManager;

class BasePass {
 protected:
  PassManager& manager_;

 public:
  explicit BasePass(PassManager& manager) : manager_(manager) {}

  virtual void apply() = 0;

  virtual ~BasePass() = default;
};
}  // namespace Passes
