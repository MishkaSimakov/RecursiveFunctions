#pragma once

#include <algorithm>
#include <concepts>
#include <iostream>
#include <string_view>
#include <type_traits>

namespace PipelineDetails {
// String Literal for passing strings as NTTP
template <size_t N>
struct StringLiteral {
  constexpr StringLiteral(const char (&str)[N]) { std::copy_n(str, N, value); }

  char value[N];

  constexpr bool operator==(std::string_view other) const {
    return static_cast<const char*>(value) == other;
  }
};

// Zip With Offset
template <typename... Args>
struct TypeListStorage {};

template <typename T, typename... Args>
struct ZipWithOffsetHelper;

template <typename... PrefixArgs, typename... Args>
struct ZipWithOffsetHelper<TypeListStorage<PrefixArgs...>, Args...> {
  using type = TypeListStorage<PrefixArgs...>;
};

template <typename... PrefixArgs, typename First, typename Second,
          typename... Tail>
struct ZipWithOffsetHelper<TypeListStorage<PrefixArgs...>, First, Second,
                           Tail...> {
  using type = typename ZipWithOffsetHelper<
      TypeListStorage<PrefixArgs..., std::pair<First, Second>>, Second,
      Tail...>::type;
};

template <typename... Args>
struct ZipWithOffset {
  using type = typename ZipWithOffsetHelper<TypeListStorage<>, Args...>::type;
};

// Correctness checks
template <typename T>
concept PipelineStageHasCorrectApplyMethod =
    (std::is_void_v<typename T::input> ? requires(T t) {
      { t.apply() } -> std::convertible_to<typename T::output>;
    } : requires(T t, typename T::input input) {
      { t.apply(std::move(input)) } -> std::convertible_to<typename T::output>;
    });

template <typename T>
concept PipelineStageHasCorrectName =
    std::is_convertible_v<decltype(T::name), std::string_view>;

template <typename U, typename V>
concept PipelineStagesAreCompatible =
    std::is_convertible_v<typename U::output, typename V::input>;

template <typename T>
struct CheckPipelineStagesCompatible : std::bool_constant<false> {};

template <typename... Pairs>
struct CheckPipelineStagesCompatible<TypeListStorage<Pairs...>>
    : std::bool_constant<(
          PipelineStagesAreCompatible<typename Pairs::first_type,
                                      typename Pairs::second_type> &&
          ...)> {};

template <typename... Args>
concept PipelineStagesCorrect =
    (PipelineStageHasCorrectApplyMethod<Args> && ...) &&
    (PipelineStageHasCorrectName<Args> && ...) &&
    CheckPipelineStagesCompatible<typename ZipWithOffset<Args...>::type>::value;

template <typename... Args>
  requires PipelineStagesCorrect<Args...>
struct PipelineStorage;

// Replace Pipeline Stage
template <typename Before, StringLiteral name, typename Replacement,
          typename After>
struct ReplacePipelineStage;

template <typename... ArgsBefore, StringLiteral name, typename Replacement,
          typename Head, typename... Tail>
struct ReplacePipelineStage<PipelineStorage<ArgsBefore...>, name, Replacement,
                            PipelineStorage<Head, Tail...>> {
  using type = decltype([] {
    if constexpr (name == Head::name) {
      return std::declval<
          PipelineStorage<ArgsBefore..., Replacement, Tail...>>();
    } else {
      return std::declval<typename ReplacePipelineStage<
          PipelineStorage<ArgsBefore..., Head>, name, Replacement,
          PipelineStorage<Tail...>>::type>();
    }
  }());
};

// Pop Front
template <typename... Args>
struct PopFront;

template <>
struct PopFront<> {
  using type = PipelineStorage<>;
};

template <typename Head, typename... Tail>
struct PopFront<Head, Tail...> {
  using type = PipelineStorage<Tail...>;
};

template <typename Before, StringLiteral name, typename... Args>
struct TakeBefore;

template <typename... Before, StringLiteral name, typename Head,
          typename... Tail>
struct TakeBefore<PipelineStorage<Before...>, name, Head, Tail...> {
  using type = decltype([] {
    using NewBeforeT = PipelineStorage<Before..., Head>;

    if constexpr (name == Head::name) {
      return std::declval<NewBeforeT>();
    } else {
      return std::declval<
          typename TakeBefore<NewBeforeT, name, Tail...>::type>();
    }
  }());
};

template <StringLiteral name, typename Head, typename... Tail>
struct TakeAfter {
  using type = decltype([] {
    if constexpr (name == Head::name) {
      return std::declval<PipelineStorage<Head, Tail...>>();
    } else {
      return std::declval<typename TakeAfter<name, Tail...>::type>();
    }
  }());
};
}  // namespace PipelineDetails

template <typename... Args>
  requires PipelineDetails::PipelineStagesCorrect<Args...>
struct PipelineStorage {
  template <typename T>
  using addOperation = PipelineStorage<Args..., T>;

  template <size_t I>
  using element = std::tuple_element_t<I, std::tuple<Args...>>;

  constexpr static size_t size = sizeof...(Args);

  template <PipelineDetails::StringLiteral name, typename Replacement>
  using replace = typename PipelineDetails::ReplacePipelineStage<
      PipelineStorage<>, name, Replacement, PipelineStorage<Args...>>::type;

  using pop_front = typename PipelineDetails::PopFront<Args...>::type;

  template <PipelineDetails::StringLiteral name>
  using take_before = typename PipelineDetails::TakeBefore<PipelineStorage<>,
                                                           name, Args...>::type;

  template <PipelineDetails::StringLiteral name>
  using take_after = typename PipelineDetails::TakeAfter<name, Args...>::type;
};

//
template <typename T>
class PipelineManager;

template <typename Head, typename... Tail>
class PipelineManager<PipelineStorage<Head, Tail...>> {
  using Storage = PipelineStorage<Head, Tail...>;
  using LastT = typename Storage::template element<Storage::size - 1>;

 public:
  static typename LastT::output apply()
    requires std::is_void_v<typename Head::input>
  {
    std::cout << "Applying: " << Head::name << "\n";

    if constexpr (std::is_void_v<typename Head::output>) {
      Head().apply();

      if constexpr (sizeof...(Tail) >= 1) {
        return PipelineManager<PipelineStorage<Tail...>>::apply();
      }
    } else {
      auto first_result = Head().apply();

      if constexpr (sizeof...(Tail) >= 1) {
        return PipelineManager<PipelineStorage<Tail...>>::apply(
            std::move(first_result));
      } else {
        return first_result;
      }
    }
  }

  template <typename T>
  static typename LastT::output apply(T&& input)
    requires(!std::is_void_v<typename Head::input>) &&
            std::convertible_to<T, typename Head::input>
  {
    std::cout << "Applying: " << Head::name << "\n";

    if constexpr (std::is_void_v<typename Head::output>) {
      Head().apply(std::forward<T>(input));

      if constexpr (sizeof...(Tail) >= 1) {
        return PipelineManager<PipelineStorage<Tail...>>::apply();
      }
    } else {
      auto first_result = Head().apply(std::forward<T>(input));

      if constexpr (sizeof...(Tail) >= 1) {
        return PipelineManager<PipelineStorage<Tail...>>::apply(
            std::move(first_result));
      } else {
        return first_result;
      }
    }
  }
};
