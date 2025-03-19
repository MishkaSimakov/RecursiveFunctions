#pragma once

#include <algorithm>
#include <numeric>
#include <type_traits>
#include <unordered_set>

static auto getRange(size_t start, size_t end) {
  std::unordered_set<size_t> result;

  for (; start != end; ++start) {
    result.insert(start);
  }

  return result;
}

namespace RegistersInfo {
inline const size_t kReturnRegister = 0;

inline const auto kBasicRegisters = getRange(0, 12);
inline const auto kSpillTemporaryRegisters = getRange(13, 15);
inline const auto kCalleSavedRegisters = getRange(19, 28);
}  // namespace RegistersInfo
