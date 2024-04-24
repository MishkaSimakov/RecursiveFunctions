#include "ProgramRunTestCase.h"

TEST_F(ProgramRunTestCase, test_simple_argmin) {
  ASSERT_EQ(run_program(get_function_call("argmin", "*")), 0);
}

TEST_F(ProgramRunTestCase, test_argmin_with_substraction) {
  ASSERT_EQ(
      run_program(get_function_call("argmin", "absolute_difference(10, *)")),
      10);
}

TEST_F(ProgramRunTestCase, test_argmin_with_greater) {
  ASSERT_EQ(run_program(get_function_call("argmin", "is_greater(50, *)")), 50);
}

TEST_F(ProgramRunTestCase, test_argmin_inside_recursive) {
  string base = "f(0) = 0;";
  base += "f(x + 1) = argmin(absolute_difference(x, *));";

  string first = base + get_function_call("f", 0);
  ASSERT_EQ(run_program(first), 0);

  string second = base + get_function_call("f", 5);
  ASSERT_EQ(run_program(second), 4);

  string third = base + get_function_call("f", 100);
  ASSERT_EQ(run_program(third), 99);
}
