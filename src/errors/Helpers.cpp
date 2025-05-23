#include "Helpers.h"

#include <iostream>

void unreachable(const char* message) {
  std::cerr << "Reached unreachable: " << message << std::endl;

  // TODO: maybe use std::unreachable in production
  abort();
}

void not_implemented(const char* message, std::source_location location) {
  std::cerr << location.file_name() << ":" << location.line() << std::endl;
  std::cerr << "Not implemented";
  if (message != nullptr) {
    std::cerr << ": " << message;
  }

  std::cerr << std::endl;
  abort();
}
