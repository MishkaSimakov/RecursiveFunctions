#include "Helpers.h"

#include <iostream>

void unreachable(const char* message) {
  std::cout << "Reached unreachable: " << message << std::endl;

  // TODO: maybe use std::unreachable in production
  abort();
}
