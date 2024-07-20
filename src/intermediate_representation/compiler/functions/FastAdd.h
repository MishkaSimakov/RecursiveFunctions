#pragma once

#include <list>

#include "compilation/Instructions.h"

using std::list;

namespace IR {
// clang-format off
inline list fast_add_instructions = {
  AssemblyInstruction("add", "x0", "x0", "x1"),
};
// clang-format on
}  // namespace IR
