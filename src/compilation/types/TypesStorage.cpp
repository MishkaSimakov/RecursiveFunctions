#include "TypesStorage.h"

Type* TypesStorage::get_type_ptr(std::unique_ptr<Type> type, size_t module_id) {
  auto [itr, was_emplaced] = types.emplace(std::move(type), module_id);
  return itr->first.get();
}
