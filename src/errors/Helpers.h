#pragma once
#include <source_location>

[[noreturn]] void unreachable(const char* message);

[[noreturn]] void not_implemented(
    const char* message = nullptr,
    std::source_location location = std::source_location::current());
