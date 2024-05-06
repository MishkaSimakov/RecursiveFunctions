#include "DebugBytecodeExecutor.h"

void DebugBytecodeExecutor::help_command() {
  const auto help =
      "List of debugger commands:\n\n"
      "breakpoint <line> -- set breakpoint on given line.\n"
      "list <line> -- print code near <line>.\n"
      "run -- continue execution of program.\n"
      "print <stack>[<from>:<to>] -- print given stack values.\n"
      "step -- execute current program instruction.\n";

  std::cout << help << std::flush;
}
