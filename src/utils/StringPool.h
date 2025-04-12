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
    return StringId(itr);
  }

  std::string_view get_string(StringId index) const { return *index.itr_; }
};
