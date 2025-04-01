#pragma once

#include <bitset>

struct DeclarationSpecifiers {
private:
  constexpr static size_t kExportedIndex = 0;
  constexpr static size_t kExternIndex = 1;

  std::bitset<2> specifiers_{0};

public:
  DeclarationSpecifiers() = default;

  DeclarationSpecifiers& set_exported(bool is_exported) {
    specifiers_[kExportedIndex] = is_exported;
    return *this;
  }

  DeclarationSpecifiers& set_extern(bool is_extern) {
    specifiers_[kExternIndex] = is_extern;
    return *this;
  }

  bool is_exported() const { return specifiers_[kExportedIndex]; }

  bool is_extern() const { return specifiers_[kExternIndex]; }
};