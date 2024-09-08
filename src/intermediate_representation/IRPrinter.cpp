#include "IRPrinter.h"

#include "passes/PassManager.h"
#include "passes/print/PrintPass.h"

std::ostream& IR::operator<<(std::ostream& os, const IRPrinter& printer) {
  Passes::PassManager pass_manager;

  auto config = Passes::PrintPassConfig{false, false, false};
  pass_manager.register_pass<Passes::PrintPass>(os, config);
  pass_manager.apply(printer.program_);

  return os;
}
