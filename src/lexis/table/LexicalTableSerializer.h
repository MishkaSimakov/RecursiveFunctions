#pragma once
#include "lexis/table/LexicalAutomatonState.h"
#include "utils/Serializer.h"

namespace Lexis {
class LexicalTableSerializer : public Serializer {
 public:
  static void serialize(std::ostream& os,
                        const std::vector<JumpTableT>& states);
};
}  // namespace Lexis
