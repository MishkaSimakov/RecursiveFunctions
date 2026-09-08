#pragma once

#include <functional>

template <typename F>
class Defer {
  std::optional<F> callback_;

 public:
  explicit Defer(F callback) : callback_(std::move(callback)) {}

  Defer(const Defer&) = delete;
  Defer& operator=(const Defer&) = delete;

  Defer(Defer&& other) noexcept
      : callback_(std::exchange(other.callback_, std::nullopt)) {}

  Defer& operator=(Defer&&) = delete;

  ~Defer() {
    if (callback_) {
      std::invoke(*callback_);
    }
  }
};
