#include "TypesStorage.h"

#include <ranges>

TypesStorage::~TypesStorage() {
  for (auto type : types) {
    delete type;
  }
}
