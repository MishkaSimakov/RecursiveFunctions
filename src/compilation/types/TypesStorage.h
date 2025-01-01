#pragma once

#include <unordered_set>

#include "ast/Nodes.h"
#include "utils/Hashers.h"

class TypesStorage {
  constexpr static auto hasher_fn =
      [](const std::pair<std::unique_ptr<Type>, size_t>& value) {
        return tuple_hasher_fn(value.first, value.second);
      };

  // type + origin module id
  std::unordered_set<std::pair<std::unique_ptr<Type>, size_t>,
                     decltype(hasher_fn)>
      types;

public:
  Type* get_type_ptr(std::unique_ptr<Type> type, size_t module_id);
};
