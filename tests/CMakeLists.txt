include(GoogleTest)
enable_testing()

# Add all files from tests directory
file(GLOB_RECURSE TestsSources "*.cpp" "*.h")
add_executable(Tests ${TestsSources})

target_compile_options(Tests PUBLIC -fsanitize=address -O0)
target_link_options(Tests PUBLIC -fsanitize=address)
target_link_libraries(Tests TeaLang LexisTools GTest::gtest_main)

gtest_discover_tests(Tests)

add_custom_command(
        TARGET Tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/programs
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/tests/feature/programs/*
        ${CMAKE_CURRENT_BINARY_DIR}/programs
        COMMENT "Cloning test programs files from: ${CMAKE_SOURCE_DIR}/tests/feature/programs/* to ${CMAKE_CURRENT_BINARY_DIR}/programs"
)

add_custom_command(
        TARGET Tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/files
        ${CMAKE_CURRENT_BINARY_DIR}/files
)