#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>

using std::string;

class Logger {
 public:
  enum Category : int {
    PREPROCESSOR = 1 << 0,
    LEXIS = 1 << 1,
    SYNTAX = 1 << 2,
    EXECUTION = 1 << 3,
    ALL = PREPROCESSOR | LEXIS | SYNTAX | EXECUTION
  };

 private:
  static Logger* instance;
  static int enabled_categories;

  template <typename... Args>
  static void print_pack(const string& separator, Args&&... args) {
    ((std::cout << separator << args), ...);
    std::cout << std::endl;
  }

  static bool should_print_category(Category category) {
    return (enabled_categories & category) != 0;
  }

 public:
  Logger() = delete;

  template <typename... Args>
  static void preprocessor(Args&&... args) {
    if (should_print_category(PREPROCESSOR)) {
      print_pack(" ", "Preprocessor says:", std::forward<Args>(args)...);
    }
  }

  template <typename... Args>
  static void lexis(Args&&... args) {
    if (should_print_category(LEXIS)) {
      print_pack(" ", "Lexis says:", std::forward<Args>(args)...);
    }
  }

  template <typename... Args>
  static void syntax(Args&&... args) {
    if (should_print_category(SYNTAX)) {
      print_pack(" ", "Syntax says:", std::forward<Args>(args)...);
    }
  }

  template <typename... Args>
  static void execution(Args&&... args) {
    if (should_print_category(EXECUTION)) {
      print_pack(" ", "Execution says:", std::forward<Args>(args)...);
    }
  }

  static void disable_category(Category category) {
    enabled_categories &= ~category;
  }

  static void enable_category(Category category) {
    enabled_categories |= category;
  }
};

inline int Logger::enabled_categories = Logger::Category::ALL;

#endif  // LOGGER_H
