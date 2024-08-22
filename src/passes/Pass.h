#pragma once
#include <string>

namespace Passes {
class PassManager;

struct PassInfo {
  std::string name{};
  bool repeat_while_changing{false};
  bool require_ssa{true};
  bool preserve_ssa{true};
};

class BasePass {
 protected:
  PassManager& manager_;
  PassInfo info_;

 public:
  explicit BasePass(PassManager& manager, const PassInfo& info = {})
      : manager_(manager), info_(info) {}

  const PassInfo& get_info() const { return info_; }

  virtual void apply() = 0;

  virtual ~BasePass() = default;
};
}  // namespace Passes
