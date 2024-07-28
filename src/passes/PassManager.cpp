#include "PassManager.h"

void Passes::PassManager::apply() {
  for (auto& pass_factory : pass_factories_) {
    auto pass = pass_factory(*this);

    pass->apply();
  }
}
