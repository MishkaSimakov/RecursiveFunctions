#include "BytecodeCompiler.h"

namespace Compilation {
void BytecodeCompiler::compile(const ProgramNode& node) {
  vector<list<Instruction>> compiled_statements;

  for (auto& statement : node.functions) {
    statement->accept(*this);

    compiled_statements.push_back(std::move(result_));
    result_.clear();
  }


}
}  // namespace Compilation