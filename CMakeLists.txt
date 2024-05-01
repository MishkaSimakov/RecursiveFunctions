cmake_minimum_required(VERSION 3.22)
project(RecursiveFunctions)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "-fsanitize=address")
#set(CMAKE_CXX_FLAGS "-O3")

# libraries
add_subdirectory(lib/argparse)
add_subdirectory(lib/fmt)

add_subdirectory(src)
target_link_libraries(RecursiveFunctions fmt::fmt)

add_executable(Cli main.cpp)
target_include_directories(Cli PUBLIC src)
target_link_libraries(Cli argparse::argparse)
target_link_libraries(Cli RecursiveFunctions)

set_target_properties(Cli
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
        OUTPUT_NAME recursive
)

add_subdirectory(tests)