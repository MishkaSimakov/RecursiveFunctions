#include <gtest/gtest.h>

#include "utils/SmartEnum.h"

ENUM(Colors, RED, GREEN, BLUE);
ENUM(ColorsWithComma, RED, GREEN, BLUE, YELLOW, );

template <typename T>
void check_enum_to_string(std::vector<std::string_view> expected) {
  std::vector<std::string_view> result;
  for (auto value : T::values) {
    result.push_back(T(value).to_string());
  }

  ASSERT_EQ(result, expected);
}

TEST(SmartEnumTests, test_enum_to_string) {
  check_enum_to_string<Colors>({"RED", "GREEN", "BLUE"});
  check_enum_to_string<ColorsWithComma>({"RED", "GREEN", "BLUE", "YELLOW"});
}

TEST(SmartEnumTests, test_in_method) {
  Colors color = Colors::RED;

  ASSERT_TRUE((color.in<Colors::RED, Colors::GREEN, Colors::BLUE>()));
  ASSERT_TRUE((color.in<Colors::RED, Colors::BLUE>()));
  ASSERT_TRUE((color.in<Colors::RED>()));

  ASSERT_FALSE((color.in<Colors::GREEN, Colors::BLUE>()));
  ASSERT_FALSE((color.in<Colors::BLUE>()));
}
