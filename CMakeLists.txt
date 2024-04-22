cmake_minimum_required(VERSION 3.28)
project(RecursiveFunctions)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "-fsanitize=address")
#set(CMAKE_CXX_FLAGS "-O3")

add_subdirectory(src)

add_executable(Example main.cpp)
target_link_libraries(Example RecursiveFunctions)

add_subdirectory(tests)
target_link_libraries(Tests RecursiveFunctions)