include(GoogleTest)
enable_testing()

# here we store all programs/libraries/files that are required during tests execution
set(TESTS_RUNTIME_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Add all files from tests directory
file(GLOB_RECURSE TESTS_SOURCES "*.cpp" "*.h")
add_executable(Tests ${TESTS_SOURCES})

target_compile_options(Tests PUBLIC -fsanitize=address -O0)
target_link_options(Tests PUBLIC -fsanitize=address)
target_link_libraries(Tests TeaLang lexis_table_tools GTest::gtest_main)

get_target_property(TEA_COMPILER_DIR cli RUNTIME_OUTPUT_DIRECTORY)
add_custom_command(
        COMMAND ${CMAKE_COMMAND} -E copy ${TEA_COMPILER_DIR}/cli ${TESTS_RUNTIME_DIR}
        DEPENDS ${TEA_COMPILER_DIR}/cli
        OUTPUT ${TESTS_RUNTIME_DIR}/cli
        COMMENT "Cloning TeaLang compiler to tests runtime directory"
)
target_compile_definitions(Tests PUBLIC TESTS_RUNTIME_DIR="${TESTS_RUNTIME_DIR}")

gtest_discover_tests(Tests)

set(TESTS_PROGRAMS_DIR ${CMAKE_SOURCE_DIR}/tests/feature/programs)

# we create separate file for communication between feature test programs and testing system
set(TESTS_PROGRAMS_COMMUNICATION_FILE "${TESTS_RUNTIME_DIR}/channel")

add_library(TeaProgramTestingFramework STATIC feature/library/testing.cpp)
target_compile_definitions(TeaProgramTestingFramework PUBLIC
        TESTS_PROGRAMS_COMMUNICATION_FILE="${TESTS_PROGRAMS_COMMUNICATION_FILE}")
set_target_properties(TeaProgramTestingFramework PROPERTIES LIBRARY_OUTPUT_DIR ${TESTS_RUNTIME_DIR})
set_target_properties(TeaProgramTestingFramework PROPERTIES OUTPUT_NAME "library")

add_custom_command(
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${TESTS_PROGRAMS_DIR} ${TESTS_RUNTIME_DIR}/programs
        DEPENDS ${TESTS_PROGRAMS_DIR}
        OUTPUT ${TESTS_RUNTIME_DIR}/programs
        COMMENT "Cloning test programs files to tests runtime directory"
)

add_custom_command(
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/files ${TESTS_RUNTIME_DIR}/files
        DEPENDS ${CMAKE_SOURCE_DIR}/files
        OUTPUT ${TESTS_RUNTIME_DIR}/files
        COMMENT "Cloning compiler files to tests runtime directory"
)

add_custom_target(
        CopyTestFiles DEPENDS
        ${TESTS_RUNTIME_DIR}/cli
        ${TESTS_RUNTIME_DIR}/programs
        ${TESTS_RUNTIME_DIR}/files
)

add_dependencies(Tests CopyTestFiles)
target_link_libraries(Tests TeaProgramTestingFramework)