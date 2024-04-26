cmake_minimum_required(VERSION 3.28)
project(RecursiveFunctions)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "-fsanitize=address")
#set(CMAKE_CXX_FLAGS "-O3")

# argparse library
add_subdirectory(lib/argparse)

add_subdirectory(src)

add_executable(Example main.cpp)
target_link_libraries(Example argparse::argparse)
target_link_libraries(Example RecursiveFunctions)

set_target_properties(Example
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

add_subdirectory(tests)