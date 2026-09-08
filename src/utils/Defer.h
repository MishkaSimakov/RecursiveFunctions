#pragma once

#include <functional>

template <typename F>
class Defer {
  F callback_;

 public:
  explicit Defer(F callback) : callback_(std::move(callback)) {}

  ~Defer() { std::invoke(callback_); }
};
