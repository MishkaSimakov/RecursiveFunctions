#pragma once

#include "compilation/StringId.h"
#include "compilation/types/Type.h"

namespace Front {
// Symbols Table Entry
// it can represent:
// 1. variable name
// 2. named type (class, typedef etc.)
// 3. function name

struct SymbolInfo {
  Type* type;
  bool is_exported;
};

struct Scope {
  std::vector<std::unique_ptr<Scope>> children;
  Scope* parent{nullptr};

  std::unordered_map<StringId, SymbolInfo> symbols;

  bool add_symbol(StringId string_id, Type* type, bool is_exported) {
    auto [itr, was_emplaced] =
        symbols.emplace(string_id, SymbolInfo{type, is_exported});
    return was_emplaced;
  }
};
}  // namespace Front
