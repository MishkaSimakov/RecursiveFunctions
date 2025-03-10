#pragma once

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

  void fire(std::string_view method_name) {
    if (was_shot_) {
      throw std::runtime_error(
          fmt::format("Method {:?} must be invoked only once for one object.",
                      method_name));
    }

    was_shot_ = true;
  }
};
