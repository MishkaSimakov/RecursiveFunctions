#include <gtest/gtest.h>

#include <deque>

#include "compilation/types/Type.h"
#include "compilation/types/TypesStorage.h"

using namespace Front;

TEST(TypesTests, test_type_hash) {
  TypesStorage storage;

  std::vector<Type*> layer;
  layer.push_back(storage.make_type<VoidType>());
  layer.push_back(storage.make_type<IntType>());
  layer.push_back(storage.make_type<BoolType>());
  layer.push_back(storage.make_type<CharType>());

  // add three layers of indirection
  for (size_t i = 0; i < 5; ++i) {
    std::vector<Type*> next_layer;

    for (Type* type : layer) {
      next_layer.push_back(storage.add_pointer(type));
    }

    layer = std::move(next_layer);
  }

  // all combinations of functions with one parameter and return type
  std::vector<Type*> types(storage.types_cbegin(), storage.types_cend());
  for (Type* parameter_type : types) {
    for (Type* return_type : types) {
      storage.make_type<FunctionType>(std::vector{parameter_type}, return_type);
    }
  }

  // some other function types
  storage.make_type<FunctionType>(
      std::vector<Type*>{storage.make_type<IntType>(),
                         storage.make_type<BoolType>(),
                         storage.make_type<VoidType>()},
      storage.make_type<VoidType>());

  // check that all hashes are different
  size_t types_count = 0;
  std::set<size_t> hash_values;
  for (auto itr = storage.types_cbegin(); itr != storage.types_cend(); ++itr) {
    hash_values.insert((*itr)->hash());
    ++types_count;
  }

  ASSERT_EQ(hash_values.size(), types_count);
}

