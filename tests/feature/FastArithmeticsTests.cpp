#include "ProgramRunTestCase.h"

TEST_F(ProgramRunTestCase, test_fast_add) {
  for (size_t i = 0; i < 10; ++i) {
    for (size_t j = 0; j < 10; ++j) {
      string program = get_function_call("__add", i, j);

      ASSERT_EQ(run_program(program), i + j);
    }
  }

  ASSERT_EQ(run_program(get_function_call("__add", 100, 100)), 200);
  ASSERT_EQ(run_program(get_function_call("__add", 1000, 500)), 1500);
  ASSERT_EQ(run_program(get_function_call("__add", 500, 1000)), 1500);
}

TEST_F(ProgramRunTestCase, test_fast_absolute_difference) {
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 10; ++j) {
      string program = get_function_call("__abs_diff", i, j);

      ASSERT_EQ(run_program(program, true), std::abs(i - j));
    }
  }

  ASSERT_EQ(run_program(get_function_call("__abs_diff", 100, 100)), 0);
  ASSERT_EQ(run_program(get_function_call("__abs_diff", 1000, 500)), 500);
  ASSERT_EQ(run_program(get_function_call("__abs_diff", 500, 1000)), 500);
}