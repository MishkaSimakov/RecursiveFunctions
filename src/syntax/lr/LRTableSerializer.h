#pragma once

#include <vector>

#include "LRTableBuilder.h"

namespace Syntax {
class LRTableSerializer {
 public:
  using ActionsTableT = std::vector<std::vector<Action>>;
  using GotoTableT = std::vector<std::vector<size_t>>;

  static void serialize(std::ostream& os, const ActionsTableT& actions_table,
                        const GotoTableT& goto_table);

  static std::pair<ActionsTableT, GotoTableT> deserialize(std::istream& is);
};
}  // namespace Syntax
