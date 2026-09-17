cmake_minimum_required(VERSION 3.22)
project(TeaLang VERSION 0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

option(TLANG_TEST "Generate the test target." ON)

# load libraries
include(load_libraries)
include(tablegen)

add_subdirectory(src/lexis)
add_subdirectory(src/syntax)

add_subdirectory(src)
add_subdirectory(std)

# add executables from cli directory
file(GLOB_RECURSE CLI_SOURCES
        "src/cli/*.cpp"
        "src/cli/*.h"
)

add_executable(tlang ${CLI_SOURCES})
target_link_libraries(tlang PUBLIC TeaLang)

if (TLANG_TEST)
    set(INSTALL_GTEST OFF)
    add_subdirectory(${CMAKE_SOURCE_DIR}/lib/googletest EXCLUDE_FROM_ALL)

    add_subdirectory(tests)
endif ()

include(install_cli)