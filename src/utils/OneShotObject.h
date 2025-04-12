#pragma once

#include <source_location>
#include <stdexcept>
#include <string_view>

#include "fmt/format.h"

class OneShotObject {
 private:
  bool was_shot_{false};

 protected:
  OneShotObject() = default;

  OneShotObject(const OneShotObject&) = delete;
  OneShotObject& operator=(const OneShotObject&) = delete;

  OneShotObject(OneShotObject&&) = delete;
  OneShotObject& operator=(OneShotObject&&) = delete;

  void fire(std::source_location location) {
    if (was_shot_) {
      throw std::runtime_error(
          fmt::format("Method {:?} must be invoked only once for one object.",
                      location.function_name()));
    }

    was_shot_ = true;
  }
};

// For some reason source_location::curent() doesn't capture the location when
// used as member default argument. But do capture when it is funtion argument:
// https://en.cppreference.com/w/cpp/utility/source_location/current
#define OSO_FIRE() fire(std::source_location::current())
