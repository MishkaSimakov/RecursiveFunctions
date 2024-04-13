#ifndef CONCATENATE_H
#define CONCATENATE_H

#include <concepts>
#include <string>

template <typename Head, typename... Tail>
  requires std::convertible_to<Head, std::string>
std::string concatenate(Head head, Tail... tail) {
  return string(head) + " " + concatenate(tail...);
}

template <typename Head, typename... Tail>
std::string concatenate(Head head, Tail... tail) {
  return std::to_string(head) + " " + concatenate(tail...);
}

inline std::string concatenate() { return ""; }

#endif  // CONCATENATE_H
