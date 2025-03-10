cmake_minimum_required(VERSION 3.22)
project(TeaLang)

set(CMAKE_CXX_STANDARD 23)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "-O3")
else ()
    set(CMAKE_CXX_FLAGS "-fsanitize=address")
endif ()

#find_program(CCACHE_FOUND "ccache")
#if (CCACHE_FOUND)
#    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
#endif ()

add_compile_definitions(BASE_PATH=\"${CMAKE_SOURCE_DIR}\")

# libraries
add_subdirectory(lib/argparse EXCLUDE_FROM_ALL)

# compile fmt as static library
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
add_subdirectory(lib/fmt EXCLUDE_FROM_ALL)

# -- llvm --
list(APPEND CMAKE_PREFIX_PATH
        /opt/homebrew/Cellar/llvm/19.1.7_1/
)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

llvm_map_components_to_libnames(llvm_libs support core irreader linker)
# -- end of llvm --

add_subdirectory(src)
target_link_libraries(TeaLang fmt::fmt argparse::argparse ${llvm_libs})
target_include_directories(TeaLang PUBLIC ${LLVM_INCLUDE_DIRS})

# add executable for compiling grammar and lexis
add_subdirectory(src/syntax)
add_subdirectory(src/lexis)

# add executables from cli directory
file(GLOB_RECURSE CliFiles
        "src/cli/*.cpp"
        "src/cli/*.h"
)
add_executable(Cli main.cpp)
target_sources(Cli PUBLIC ${CliFiles})

target_include_directories(Cli PUBLIC src)
target_link_libraries(Cli TeaLang)

# copy compiler files into build destination
add_custom_command(
        TARGET Cli POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/files
        ${CMAKE_CURRENT_BINARY_DIR}/files)

add_subdirectory(tests EXCLUDE_FROM_ALL)

set_target_properties(Cli PROPERTIES OUTPUT_NAME "tlang")

if (COMMAND install)
    install(TARGETS Cli)
else ()
    set_target_properties(Cli
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    )

endif ()