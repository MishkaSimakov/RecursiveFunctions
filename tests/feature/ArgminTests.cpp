#include "ProgramRunTestCase.h"

TEST_F(ProgramRunTestCase, simple_argmin_test) {
  ASSERT_EQ(run_program(get_function_call("argmin", "*")), 0);
}