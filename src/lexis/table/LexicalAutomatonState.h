#pragma once

#include <variant>
#include <vector>

#include "lexis/Charset.h"
#include "lexis/Token.h"
#include "utils/Hashers.h"

namespace Lexis {
struct NextStateJump {
  size_t state_id;

  bool operator==(const NextStateJump&) const = default;
};

struct FinishJump {
  size_t forward_shift;
  TokenType token;

  bool operator==(const FinishJump&) const = default;
};

struct RejectJump {
  bool operator==(const RejectJump&) const = default;
};

using StatesMappingT = std::array<ssize_t, TokenType::count>;
using JumpT = std::variant<NextStateJump, FinishJump, RejectJump>;
using JumpTableT = std::array<JumpT, Charset::kCharactersCount>;

inline constexpr auto states_hasher_fn = [](const StatesMappingT& mapping) {
  StreamHasher hasher{};
  for (long value : mapping) {
    hasher << value;
  }
  return hasher.get_hash();
};
}  // namespace Lexis
