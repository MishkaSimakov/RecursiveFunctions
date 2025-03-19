#pragma once

#include <string>

namespace Assembly {
class AssemblyInstruction {
  std::string instruction_;

 public:
  template <typename... Args>
  AssemblyInstruction(std::string instruction, Args&&... strings)
      : instruction_(instruction + " " + ((std::string(strings) + ", ") + ...)) {
    instruction_.pop_back();
    instruction_.pop_back();
  }

  AssemblyInstruction(std::string instruction) : instruction_(instruction) {}

  std::string get_string() const { return instruction_; }
};
}  // namespace Assembly
