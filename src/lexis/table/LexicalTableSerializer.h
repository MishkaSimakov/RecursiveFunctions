#pragma once

#include "lexis/table/LexicalAutomatonState.h"

namespace Lexis {
class LexicalTableSerializer {
 public:
  static void serialize(std::ostream& os,
                        const std::vector<JumpTableT>& states);
};
}  // namespace Lexis
