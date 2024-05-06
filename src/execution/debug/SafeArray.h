#pragma once

enum class ArrayBordersViolationType { BORDERS_OVERFLOW, BORDERS_UNDERFLOW };

template <typename T>
concept WrappableContainer =
    std::is_default_constructible_v<T> && requires(T container, size_t index) {
      { container.size() } -> std::same_as<size_t>;
      typename T::reference;
      { container[index] } -> std::same_as<typename T::reference>;
    };

template <WrappableContainer T, typename ExceptionThrower>
  requires std::invocable<ExceptionThrower, int, ArrayBordersViolationType>
class SafeWrapper {
  [[no_unique_address]] ExceptionThrower thrower_;
  T storage_;

  void guard_access(int index) const {
    if (index < 0) {
      throw thrower_(index, ArrayBordersViolationType::BORDERS_UNDERFLOW);
    }
    if (index >= size()) {
      throw thrower_(index, ArrayBordersViolationType::BORDERS_OVERFLOW);
    }
  }

 public:
  SafeWrapper() = default;
  explicit SafeWrapper(T&& other) : storage_(std::forward<T>(other)) {}

  size_t size() const { return storage_.size(); }

  T& wrapped() const { return storage_; }

#ifdef __cpp_explicit_this_parameter
  auto& operator[](this auto&& self, int index) {
    self.guard_access(index);
    return self.storage_[index];
  }
#else
  auto& operator[](int index) {
    guard_access(index);

    return storage_[index];
  }

  const auto& operator[](int index) const {
    return const_cast<SafeWrapper&>(*this)[index];
  }
#endif
};
