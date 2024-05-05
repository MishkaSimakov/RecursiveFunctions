cmake_minimum_required(VERSION 3.22)
project(RecursiveFunctions)

set(CMAKE_CXX_STANDARD 23)

if (NOT DEFINED ${CMAKE_BUILD_TYPE})
    set(CMAKE_BUILD_TYPE Release)
endif ()

if (${CMAKE_BUILD_TYPE} EQUAL "Release")
    set(CMAKE_CXX_FLAGS "-O3")
else ()
    set(CMAKE_CXX_FLAGS "-fsanitize=address")
endif ()

find_program(CCACHE_FOUND "ccache")
if (CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif ()

# libraries
add_subdirectory(lib/argparse EXCLUDE_FROM_ALL)

# compile fmt as static library
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
add_subdirectory(lib/fmt EXCLUDE_FROM_ALL)

add_subdirectory(src)
target_link_libraries(RecursiveFunctions fmt::fmt)

add_executable(Cli main.cpp)
target_include_directories(Cli PUBLIC src)
target_link_libraries(Cli argparse::argparse)
target_link_libraries(Cli RecursiveFunctions)

add_subdirectory(tests EXCLUDE_FROM_ALL)

set_target_properties(Cli PROPERTIES OUTPUT_NAME "recurator")

if (COMMAND install)
    install(TARGETS Cli)
else ()
    set_target_properties(Cli
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    )
endif ()