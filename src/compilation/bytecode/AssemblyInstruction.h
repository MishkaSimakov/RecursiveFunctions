#pragma once

#include <string>

using std::string;

class AssemblyInstruction {
  std::string instruction_;

 public:
  template <typename... Args>
  AssemblyInstruction(string instruction, Args&&... strings)
      : instruction_(instruction + " " + ((string(strings) + ", ") + ...)) {
    instruction_.pop_back();
    instruction_.pop_back();
  }

  AssemblyInstruction(string instruction) : instruction_(instruction) {}

  string get_string() const { return instruction_; }
};
