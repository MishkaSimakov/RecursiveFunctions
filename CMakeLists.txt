cmake_minimum_required(VERSION 3.28)
project(RecursiveFunctions)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "-fsanitize=address")

add_executable(RecursiveFunctions main.cpp
        src/preprocessor/Preprocessor.h)

target_include_directories(RecursiveFunctions PUBLIC src)