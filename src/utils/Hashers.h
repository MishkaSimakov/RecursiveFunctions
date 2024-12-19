#pragma once
#include <unordered_map>
#include <unordered_set>

inline auto hash_fn = []<typename T>(const T& value) {
  return std::hash<T>()(value);
};

struct StreamHasher {
 private:
  size_t current_ = 0;

 public:
  template <typename T>
  StreamHasher& operator<<(const T& value) {
    current_ ^=
        std::hash<T>()(value) + 0xeeffccdd + (current_ << 5) + (current_ >> 3);

    return *this;
  }

  size_t get_hash() const { return current_; }
};

template <typename... Args>
struct TupleHasher {
  size_t operator()(const Args&... args) const {
    StreamHasher hasher{};
    (hasher << ... << args);
    return hasher.get_hash();
  }
};

inline auto tuple_hasher_fn = []<typename... Args>(const Args&... args) {
  return TupleHasher<Args...>()(args...);
};

template <typename R,
          typename ElementHasher = std::hash<std::ranges::range_value_t<R>>>
struct UnorderedRangeHasher {
 private:
  static constexpr size_t cModulo = 1000003793;
  static constexpr size_t cBase = 239;

  static size_t fast_exponent(size_t value) {
    size_t result = 1;
    size_t base = cBase;

    while (value > 0) {
      if (value % 2 == 1) {
        result *= base;
        result %= cModulo;
      }

      base *= base;
      base %= cModulo;

      value /= 2;
    }

    return result;
  }

  [[no_unique_address]] ElementHasher hasher_;

 public:
  size_t operator()(const R& range) const {
    size_t result = 0;

    for (const auto& value : range) {
      size_t hash = hasher_(value);
      result ^= fast_exponent(hash);
    }

    return result;
  }
};

inline auto unordered_range_hasher_fn = []<typename R>(const R& range) {
  return UnorderedRangeHasher<R>()(range);
};
