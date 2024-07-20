#include "BytecodeCompiler.h"

namespace Compilation {
void BytecodeCompiler::visit(const ConstantNode& node) {
  string return_register = get_register(current_argument_offset);

  ssize_t value = node.value;
  constexpr size_t kMaxMovBits = 16;
  constexpr size_t kMaxMovValue = (1 << kMaxMovBits) - 1;

  if (value > kMaxMovValue) {
    size_t last_bits = value & kMaxMovValue;
    size_t first_bits = value >> kMaxMovBits;

    result_.emplace_back("mov", return_register, get_constant(last_bits));
    result_.emplace_back("movk", return_register, get_constant(first_bits),
                         "lsl " + get_constant(16));
  } else {
    result_.emplace_back("mov", return_register, get_constant(value));
  }
}
}  // namespace Compilation
