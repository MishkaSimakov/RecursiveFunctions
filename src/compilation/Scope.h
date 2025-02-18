#pragma once

#include <unordered_set>

#include "compilation/StringId.h"
#include "compilation/types/Type.h"

namespace Front {
struct SymbolInfo {
  Type* type;
  bool is_exported;
};

struct Scope {
  std::unordered_set<Scope*> children;
  Scope* parent{nullptr};

  std::unordered_map<StringId, SymbolInfo> symbols;

  bool add_symbol(StringId string_id, Type* type, bool is_exported) {
    auto [itr, was_emplaced] =
        symbols.emplace(string_id, SymbolInfo{type, is_exported});
    return was_emplaced;
  }
};
}  // namespace Front
