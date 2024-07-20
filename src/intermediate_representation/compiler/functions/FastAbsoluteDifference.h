#pragma once

#include <list>

using std::list;

namespace IR {
// clang-format off
inline list fast_absolute_difference_instructions = {
  AssemblyInstruction("subs", "x0", "x0", "x1"),
  AssemblyInstruction("cneg", "x0", "x0", "mi")
};
// clang-format on
}  // namespace IR
