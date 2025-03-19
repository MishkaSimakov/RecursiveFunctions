cmake_minimum_required(VERSION 3.22)
project(TeaLang)

set(CMAKE_CXX_STANDARD 23)
set(TABLEGEN_CMAKE_DIR ${CMAKE_SOURCE_DIR}/cmake-build-tablegen)

add_compile_definitions(BASE_PATH=\"${CMAKE_SOURCE_DIR}\")

# load libraries
include(cmake/libraries.cmake)

add_subdirectory(src)
target_link_libraries(TeaLang fmt::fmt argparse::argparse ${llvm_libs})
target_include_directories(TeaLang PUBLIC ${LLVM_INCLUDE_DIRS})

include(cmake/tablegen.cmake)
add_tablegen_cmake_directory()

add_subdirectory(src/lexis)
add_subdirectory(src/syntax)

if (BUILD_ONLY_TABLEGEN)
    return()
endif()

add_dependencies(TeaLang run_grammar_tablegen run_lexis_tablegen)

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