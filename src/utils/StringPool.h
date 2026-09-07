#pragma once

#include <set>
#include <string>

#include "StringId.h"

class StringPool {
  // I use std::less<void> to compare std::string with std::string_view without
  // creating new string from string_view
  std::set<std::string, std::less<>> strings_table_;

public:
  StringId add_string(std::string_view string) {
    auto [itr, _] = strings_table_.emplace(string);
    return StringId(itr, this);
  }

  std::string_view get_string(StringId index) const {
    if constexpr (Constants::debug) {
      assert(index.pool_identifier_ == detail::StringPoolIdentifier<true>(this));
    }
    return *index.itr_;
  }
};
