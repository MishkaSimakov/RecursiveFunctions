#include "BytecodeCompiler.h"

namespace Compilation {
vector<AssemblyInstruction> BytecodeCompiler::get_result() const {
  vector<AssemblyInstruction> result;

  for (auto& definition : compiled_functions_) {
    std::copy(definition.begin(), definition.end(), std::back_inserter(result));
  }

  std::copy(compiled_call_.begin(), compiled_call_.end(),
            std::back_inserter(result));

  return result;
}
}  // namespace Compilation
