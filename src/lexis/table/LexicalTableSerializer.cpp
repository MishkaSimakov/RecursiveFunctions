#include "LexicalTableSerializer.h"

#include "fmt/format.h"
#include "utils/IncFilesGeneration.h"
#include "utils/TupleUtils.h"

namespace Lexis {
void LexicalTableSerializer::serialize(std::ostream& os,
                                       const std::vector<JumpTableT>& states) {
  write_header(os);

  // write prologue
  os << fmt::format("constexpr size_t states_count = {};\n", states.size());
  os << fmt::format("constexpr size_t characters_count = {};\n",
                    Charset::kCharactersCount);
  os << "constexpr JumpT lexis_dfa_table[] = {\n";

  // write jumps
  for (const auto& node : states) {
    for (const auto& jump : node) {
      std::visit(Overloaded{
                     [&os](RejectJump) { os << "RejectJump()"; },
                     [&os](NextStateJump jump) {
                       os << fmt::format("NextStateJump({})", jump.state_id);
                     },
                     [&os](FinishJump jump) {
                       os << fmt::format("FinishJump({}, TokenType({}))",
                                         jump.forward_shift,
                                         static_cast<size_t>(jump.token));
                     },
                 },
                 jump);

      os << ", ";
    }

    os << "\n";
  }

  // write epilogue
  os << "};\n";
}
}  // namespace Lexis
