#include "ProgramRunTestCase.h"

TEST_F(ProgramRunTestCase, test_add) {
  for (size_t i = 0; i < 10; ++i) {
    for (size_t j = 0; j < 10; ++j) {
      string program = get_function_call("add", i, j);

      ASSERT_EQ(run_program(program), i + j);
    }
  }

  ASSERT_EQ(run_program(get_function_call("add", 1000, 1000)), 2000);
  ASSERT_EQ(run_program(get_function_call("add", 5000, 10000)), 15000);
  ASSERT_EQ(run_program(get_function_call("add", 10000, 5000)), 15000);
}

TEST_F(ProgramRunTestCase, test_multiply) {
  for (size_t i = 0; i < 10; ++i) {
    for (size_t j = 0; j < 10; ++j) {
      string program = get_function_call("multiply", i, j);

      ASSERT_EQ(run_program(program), i * j);
    }
  }

  ASSERT_EQ(run_program(get_function_call("multiply", 100, 100)), 1e4);
  ASSERT_EQ(run_program(get_function_call("multiply", 500, 1000)), 5e5);
  ASSERT_EQ(run_program(get_function_call("multiply", 1000, 500)), 5e5);
}

TEST_F(ProgramRunTestCase, test_power) {
  for (size_t i = 0; i < 8; ++i) {
    for (size_t j = 0; j < 8; ++j) {
      string program = get_function_call("power", i, j);

      ASSERT_EQ(run_program(program), std::pow(i, j));
    }
  }
}

TEST_F(ProgramRunTestCase, test_predecessor) {
  ASSERT_EQ(run_program(get_function_call("predecessor", 0)), 0);
  ASSERT_EQ(run_program(get_function_call("predecessor", 1)), 0);
  ASSERT_EQ(run_program(get_function_call("predecessor", 1000)), 999);
  ASSERT_EQ(run_program(get_function_call("predecessor", 123123)), 123122);
}

TEST_F(ProgramRunTestCase, test_monus) {
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 10; ++j) {
      string program = get_function_call("monus", i, j);

      ASSERT_EQ(run_program(program), std::max(i - j, 0));
    }
  }

  ASSERT_EQ(run_program(get_function_call("monus", 1000, 1000)), 0);
  ASSERT_EQ(run_program(get_function_call("monus", 500, 1000)), 0);
  ASSERT_EQ(run_program(get_function_call("monus", 1000, 500)), 500);
}

TEST_F(ProgramRunTestCase, test_absolute_difference) {
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 10; ++j) {
      string program = get_function_call("absolute_difference", i, j);

      ASSERT_EQ(run_program(program), std::abs(i - j));
    }
  }

  ASSERT_EQ(run_program(get_function_call("absolute_difference", 1000, 1000)),
            0);
  ASSERT_EQ(run_program(get_function_call("absolute_difference", 500, 1000)),
            500);
  ASSERT_EQ(run_program(get_function_call("absolute_difference", 1000, 500)),
            500);
}

TEST_F(ProgramRunTestCase, test_is_positive) {
  ASSERT_EQ(run_program(get_function_call("is_positive", 0)), 0);
  ASSERT_EQ(run_program(get_function_call("is_positive", 1)), 1);
  ASSERT_EQ(run_program(get_function_call("is_positive", 100)), 1);
  ASSERT_EQ(run_program(get_function_call("is_positive", 1000)), 1);
}

TEST_F(ProgramRunTestCase, test_not) {
  ASSERT_EQ(run_program(get_function_call("not", 0)), 1);
  ASSERT_EQ(run_program(get_function_call("not", 1)), 0);
}

TEST_F(ProgramRunTestCase, test_or) {
  ASSERT_EQ(run_program(get_function_call("or", 0, 0)), 0);
  ASSERT_EQ(run_program(get_function_call("or", 0, 1)), 1);
  ASSERT_EQ(run_program(get_function_call("or", 1, 0)), 1);
  ASSERT_EQ(run_program(get_function_call("or", 1, 1)), 1);
}

TEST_F(ProgramRunTestCase, test_and) {
  ASSERT_EQ(run_program(get_function_call("and", 0, 0)), 0);
  ASSERT_EQ(run_program(get_function_call("and", 0, 1)), 0);
  ASSERT_EQ(run_program(get_function_call("and", 1, 0)), 0);
  ASSERT_EQ(run_program(get_function_call("and", 1, 1)), 1);
}

TEST_F(ProgramRunTestCase, test_xor) {
  ASSERT_EQ(run_program(get_function_call("xor", 0, 0)), 0);
  ASSERT_EQ(run_program(get_function_call("xor", 0, 1)), 1);
  ASSERT_EQ(run_program(get_function_call("xor", 1, 0)), 1);
  ASSERT_EQ(run_program(get_function_call("xor", 1, 1)), 0);
}

TEST_F(ProgramRunTestCase, test_is_equal) {
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 10; ++j) {
      string program = get_function_call("is_equal", i, j);

      ASSERT_EQ(run_program(program), i == j ? 1 : 0);
    }
  }

  ASSERT_EQ(run_program(get_function_call("is_equal", 1000, 1000)), 1);
  ASSERT_EQ(run_program(get_function_call("is_equal", 500, 1000)), 0);
  ASSERT_EQ(run_program(get_function_call("is_equal", 1000, 500)), 0);
}

TEST_F(ProgramRunTestCase, test_is_greater) {
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 10; ++j) {
      string program = get_function_call("is_greater", i, j);

      ASSERT_EQ(run_program(program), i > j ? 1 : 0);
    }
  }

  ASSERT_EQ(run_program(get_function_call("is_greater", 1000, 1000)), 0);
  ASSERT_EQ(run_program(get_function_call("is_greater", 500, 1000)), 0);
  ASSERT_EQ(run_program(get_function_call("is_greater", 1000, 500)), 1);
}

TEST_F(ProgramRunTestCase, test_conditional) {
  ASSERT_EQ(run_program(get_function_call("conditional", 0, 123, 321)), 321);
  ASSERT_EQ(run_program(get_function_call("conditional", 1, 123, 321)), 123);
  ASSERT_EQ(run_program(get_function_call("conditional", 0, 111, 222)), 222);
  ASSERT_EQ(run_program(get_function_call("conditional", 1, 111, 222)), 111);
  ASSERT_EQ(run_program(get_function_call("conditional", 0, 0, 0)), 0);
  ASSERT_EQ(run_program(get_function_call("conditional", 1, 0, 0)), 0);
}

TEST_F(ProgramRunTestCase, test_remainder) {
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 10; ++j) {
      string program = get_function_call("remainder", i, j);

      ASSERT_EQ(run_program(program), i % j);
    }
  }

  ASSERT_EQ(run_program(get_function_call("remainder", 1233, 123)), 3);
  ASSERT_EQ(run_program(get_function_call("remainder", 111, 12399)), 111);
  ASSERT_EQ(run_program(get_function_call("remainder", 500, 500)), 0);
}

TEST_F(ProgramRunTestCase, test_is_divisible) {
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 10; ++j) {
      string program = get_function_call("is_divisible", i, j);

      ASSERT_EQ(run_program(program), i % j == 0 ? 1 : 0);
    }
  }

  ASSERT_EQ(run_program(get_function_call("is_divisible", 100, 100)), 1);
  ASSERT_EQ(run_program(get_function_call("is_divisible", 123, 321)), 0);
  ASSERT_EQ(run_program(get_function_call("is_divisible", 100, 500)), 0);
}


TEST_F(ProgramRunTestCase, test_sqrt) {
  for (int i = 0; i < 100; ++i) {
      string program = get_function_call("sqrt", i);

      ASSERT_EQ(run_program(program), std::floor(std::sqrt(i)));
  }
}

