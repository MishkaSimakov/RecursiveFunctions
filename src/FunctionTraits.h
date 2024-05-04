#pragma once

#include <tuple>

template <typename F>
struct FunctionTraits;

template <typename R, typename... Args>
struct FunctionTraits<R (*)(Args...)> {
  using Pointer = R (*)(Args...);
  using RetType = R;
  using ArgTypes = std::tuple<Args...>;
  static constexpr std::size_t ArgCount = sizeof...(Args);
};