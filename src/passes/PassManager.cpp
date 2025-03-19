#include "PassManager.h"

#include <sstream>

#include "intermediate_representation/IRPrinter.h"
#include "verification/VerificationPass.h"

void Passes::PassManager::apply(IR::Program& program) {
  program_ = &program;

  for (auto& pass_factory : pass_factories_) {
    auto pass = pass_factory(*this);
    const auto& info = pass->get_info();

    if (info.require_ssa && !is_in_ssa_) {
      throw std::runtime_error(fmt::format(
          "Pass \"{}\" requires program to be in SSA form but it is not so.",
          info.name));
    }

    pass->base_apply(*program_);

    if (!info.preserve_ssa) {
      is_in_ssa_ = false;
    }

#ifndef NDEBUG
    if (!should_verify_) {
      continue;
    }

    auto verification = std::make_unique<VerificationPass>(*this);

    try {
      static_cast<BasePass*>(verification.get())->base_apply(program);
    } catch (VerificationException exception) {
      std::ostringstream message;

      message << fmt::format(
          "Verification pass failed after \"{}\" pass.\nReason: {}\n", info.name,
          exception.what());

      message << "In function " << exception.function.name << ":\n";
      message << IR::IRPrinter{program};

      throw std::runtime_error(message.str());
    } catch (std::runtime_error exception) {
      throw std::runtime_error(
          fmt::format("Verification pass failed after \"{}\" pass.\nReason: {}",
                      info.name, exception.what()));
    }
#endif
  }
}
