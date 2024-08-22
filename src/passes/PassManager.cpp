#include "PassManager.h"

#include "verification/VerificationPass.h"

void Passes::PassManager::apply() {
  for (auto& pass_factory : pass_factories_) {
    auto pass = pass_factory(*this);
    const auto& info = pass->get_info();

    if (info.require_ssa && !is_in_ssa_) {
      throw std::runtime_error(fmt::format(
          "Pass \"{}\" requires program to be in SSA form but it is not so.",
          info.name));
    }

    pass->apply();

    if (!info.preserve_ssa) {
      is_in_ssa_ = false;
    }

#ifndef NDEBUG
    auto verification = std::make_unique<VerificationPass>(*this);

    try {
      static_cast<BasePass*>(verification.get())->apply();
    } catch (std::runtime_error error) {
      throw std::runtime_error(
          fmt::format("Verification pass failed after \"{}\" pass.\nReason: {}",
                      info.name, error.what()));
    }
#endif
  }
}
