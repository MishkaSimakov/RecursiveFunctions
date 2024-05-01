#pragma once

#include <concepts>
#include <exception>
#include <iostream>

#include "Exceptions.h"

class ExceptionsHandler {
 public:
  template <std::invocable<> Callable>
  static int execute(const Callable& callable) {
    try {
      callable();
    } catch (const Cli::ArgumentsParseException& exception) {
      std::cerr << "Fatal error when parsing arguments:" << std::endl;
      std::cerr << exception.what() << std::endl;
      std::cerr << "Write --help to read help message." << std::endl;

      return 1;
    } catch (const std::exception& exception) {
      std::cerr << "Fatal error:" << std::endl;
      std::cerr << exception.what() << std::endl;

      return 1;
    }

    return 0;
  }
};
