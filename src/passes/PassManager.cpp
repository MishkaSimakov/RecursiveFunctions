#include "PassManager.h"

#include "verification/VerificationPass.h"

void Passes::PassManager::apply() {
  for (auto& pass_factory : pass_factories_) {
    auto pass = pass_factory(*this);

    pass->apply();

#ifndef NDEBUG
    auto verification = std::make_unique<VerificationPass>(*this);

    try {
      static_cast<BasePass*>(verification.get())->apply();
    } catch (std::runtime_error error) {
      throw std::runtime_error(
          fmt::format("Verification pass failed after \"{}\" pass.\nReason: {}",
                      pass->get_info().name, error.what()));
    }
#endif
  }
}
