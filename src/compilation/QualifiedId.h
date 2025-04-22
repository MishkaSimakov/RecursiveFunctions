#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <initializer_list>
#include <vector>

#include "utils/Hashers.h"
#include "utils/StringPool.h"

namespace Front {
struct QualifiedId {
  std::vector<StringId> parts;

  explicit QualifiedId(std::vector<StringId> parts) : parts(std::move(parts)) {}
  QualifiedId(std::initializer_list<StringId> parts)
      : parts(std::move(parts)) {}

  bool operator==(const QualifiedId&) const = default;

  StringId unqualified_id() const { return parts.back(); }
  bool is_qualified() const { return parts.size() > 1; }

  auto qualifiers_view() const {
    return parts | std::views::take(parts.size() - 1);
  }

  size_t hash() const noexcept {
    StreamHasher hasher;
    for (StringId part : parts) {
      hasher << part;
    }
    return hasher.get_hash();
  }

  StringId pop_name() {
    StringId name = parts.back();
    parts.pop_back();
    return name;
  }

  std::string to_string(const StringPool& strings) const {
    auto string_parts = parts | std::views::transform([&strings](StringId id) {
                          return strings.get_string(id);
                        });

    return fmt::format("{}", fmt::join(string_parts, "::"));
  }
};
}  // namespace Front

template <>
struct std::hash<Front::QualifiedId> {
  size_t operator()(const Front::QualifiedId& id) const noexcept {
    return id.hash();
  }
};
