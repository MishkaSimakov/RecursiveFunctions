#pragma once
#include <type_traits>
#include <variant>

template <class... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;
};

template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

template <typename... Types>
struct type_index;

template <typename T>
struct type_index<T> {};

template <typename T, typename Head, typename... Tail>
struct type_index<T, Head, Tail...> {
  static constexpr size_t value = [] {
    if constexpr (std::is_same_v<T, Head>) {
      return 0;
    } else {
      return type_index<T, Tail...>::value + 1;
    }
  }();
};

template <typename T, typename V>
struct variant_type_index;

template <typename T, typename... Types>
struct variant_type_index<T, std::variant<Types...>> : type_index<T, Types...> {
};

template <typename T, typename V>
constexpr size_t variant_type_index_v = variant_type_index<T, V>::value;
