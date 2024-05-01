#pragma once

#include <iostream>
#include <string>

#define LOGGER_CATEGORY_LOG(category_uppercase, category_lowercase)   \
  template <typename... Args>                                         \
  static void category_lowercase(LogLevel level, Args&&... args) {    \
    if (should_print(level, category_uppercase)) {                    \
      print_pack(" ", "-",                                            \
                 format(#category_lowercase " says:", prefix_length), \
                 std::forward<Args>(args)...);                        \
    }                                                                 \
  }

using std::string;

enum class LogLevel : size_t { DEBUG = 0, INFO, WARNING };

class Logger {
 public:
  enum Category : int {
    PREPROCESSOR = 1 << 0,
    LEXIS = 1 << 1,
    SYNTAX = 1 << 2,
    EXECUTION = 1 << 3,
    ALL = PREPROCESSOR | LEXIS | SYNTAX | EXECUTION
  };

  static size_t log_level_to_number(LogLevel level) {
    return static_cast<size_t>(level);
  }

 private:
  static constexpr auto prefix_length = 20;
  static Logger* instance;
  static int enabled_categories;
  static LogLevel log_level;

  template <typename... Args>
  static void print_pack(const string& separator, Args&&... args) {
    ((std::cout << separator << args), ...);
    std::cout << std::endl;
  }

  static bool should_print(LogLevel level, Category category) {
    return log_level_to_number(level) >= log_level_to_number(log_level) &&
           (enabled_categories & category) != 0;
  }

  static string format(string value, size_t desired_length) {
    value.resize(desired_length, ' ');
    if (!value.empty()) {
      value[0] = std::toupper(value[0]);
    }

    return value;
  }

 public:
  Logger() = delete;

  LOGGER_CATEGORY_LOG(PREPROCESSOR, preprocessor);
  LOGGER_CATEGORY_LOG(LEXIS, lexis);
  LOGGER_CATEGORY_LOG(SYNTAX, syntax);
  LOGGER_CATEGORY_LOG(EXECUTION, execution);

  static void disable_category(Category category) {
    enabled_categories &= ~category;
  }

  static void enable_category(Category category) {
    enabled_categories |= category;
  }

  static void set_level(LogLevel level) { log_level = level; }

  static void set_level(size_t level) {
    if (level == 0) {
      disable_category(ALL);
    } else if (level == 1) {
      set_level(LogLevel::WARNING);
    } else if (level == 2) {
      set_level(LogLevel::INFO);
    } else {
      set_level(LogLevel::DEBUG);
    }
  }
};

inline int Logger::enabled_categories = Logger::Category::ALL;
inline LogLevel Logger::log_level = LogLevel::INFO;
