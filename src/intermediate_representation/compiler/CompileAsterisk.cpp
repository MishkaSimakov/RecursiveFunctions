#include "IRCompiler.h"

namespace IR {
void IRCompiler::visit(const AsteriskNode& node) {
  assign_or_pass_as_argument(asterisk_temporary_);
}
}  // namespace IR
