#include "LexicalTableSerializer.h"

#include "utils/TupleUtils.h"

namespace Lexis {
void LexicalTableSerializer::serialize(std::ostream& os,
                                       const std::vector<JumpTableT>& states) {
  write_bytes(states.size(), os);

  for (const auto& node : states) {
    for (const auto& jump : node) {
      write_bytes(jump.index(), os);
      std::visit(Overloaded{[](RejectJump) {},
                            [&os](NextStateJump jump) {
                              write_bytes(jump.state_id, os);
                            },
                            [&os](FinishJump jump) {
                              write_bytes(jump.forward_shift, os);
                              write_bytes(static_cast<size_t>(jump.token), os);
                            }},
                 jump);
    }
  }
}
std::vector<JumpTableT> LexicalTableSerializer::deserialize(std::istream& is) {
  size_t states_count = read_bytes(is);
  std::vector<JumpTableT> result(states_count);

  for (size_t i = 0; i < states_count; ++i) {
    for (size_t j = 0; j < Charset::kCharactersCount; ++j) {
      size_t index = read_bytes(is);
      switch (index) {
        case variant_type_index_v<RejectJump, JumpT>:
          result[i][j] = RejectJump();
          break;
        case variant_type_index_v<NextStateJump, JumpT>:
          result[i][j] = NextStateJump{read_bytes(is)};
          break;
        case variant_type_index_v<FinishJump, JumpT>:
          size_t forward_shift{read_bytes(is)};
          TokenType token{read_bytes(is)};
          result[i][j] = FinishJump{forward_shift, token};
          break;
      }
    }
  }

  return result;
}
}  // namespace Lexis
