cmake_minimum_required(VERSION 3.22)
project(TeaLang)

set(CMAKE_CXX_STANDARD 23)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(TEALANG_FILES_DIRECTORY "${CMAKE_SOURCE_DIR}/files")
# This definition is required when we build target for playground and tests
# When target is built for installation, it calculates this path in a different way
add_compile_definitions(FILES_DIRECTORY="${TEALANG_FILES_DIRECTORY}")

# load libraries
include(load_libraries)
include(tablegen)

add_subdirectory(src/lexis)
add_subdirectory(src/syntax)

add_subdirectory(src)

# add executables from cli directory
file(GLOB_RECURSE CLI_SOURCES
        "src/cli/*.cpp"
        "src/cli/*.h"
)

add_executable(cli ${CLI_SOURCES})
target_link_libraries(cli PUBLIC TeaLang)
target_compile_definitions(cli PUBLIC BUILD_FOR_INSTALLATION=false)
set_target_properties(cli PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(cli_installation ${CLI_SOURCES})
target_link_libraries(cli_installation PUBLIC TeaLang)
target_compile_definitions(cli_installation PUBLIC BUILD_FOR_INSTALLATION=true)

add_subdirectory(tests EXCLUDE_FROM_ALL)

include(install_cli)