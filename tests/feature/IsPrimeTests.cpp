#include "ProgramRunTestCase.h"

class IsPrimeTestCase : public ProgramRunTestCase {
 protected:
  bool is_prime(int number) {
    if (number == 0 || number == 1) {
      return false;
    }

    for (int i = 2; i * i <= number; ++i) {
      if (number % i == 0) {
        return false;
      }
    }

    return true;
  }
};

TEST_F(IsPrimeTestCase, test_is_prime_works) {
  for (int i = 0; i < 200; ++i) {
    ASSERT_EQ(run_program(get_function_call("is_prime", i), true),
              is_prime(i) ? 1 : 0);
  }
}

TEST_F(IsPrimeTestCase, test_is_prime_argmin_works) {
  for (int i = 0; i < 200; ++i) {
    ASSERT_EQ(run_program(get_function_call("is_prime_argmin", i), true),
              is_prime(i) ? 1 : 0);
  }
}
