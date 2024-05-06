# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif ()

include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
        Tests
        feature/ArithmeticsTests.cpp
        feature/FastArithmeticsTests.cpp
        feature/ProgramRunTestCase.h
        feature/ArgminTests.cpp
        unit/lexis/LexisTests.cpp
        unit/syntax/CorrectSyntaxTests.cpp
        unit/syntax/SyntaxTestCase.h
        unit/syntax/SyntaxNodeTests.cpp
        unit/syntax/SyntaxExceptionsTests.cpp
        unit/compilation/CompileTreeTestCase.h
        unit/compilation/CorrectCompileTreeTests.cpp
        unit/preprocessor/PreprocessorTestCase.h
        unit/preprocessor/PreprocessorTests.cpp
        unit/execution/BytecodeExecutorTests.cpp
)

target_compile_options(Tests PUBLIC -fsanitize=address -O0)

target_link_libraries(Tests RecursiveFunctions)
target_link_libraries(Tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(Tests)

add_custom_command(
        TARGET Tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/programs
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/tests/feature/programs/*
        ${CMAKE_CURRENT_BINARY_DIR}/programs
        COMMENT "Cloning test programs files from: ${CMAKE_SOURCE_DIR}/tests/feature/programs/* to ${CMAKE_CURRENT_BINARY_DIR}/programs"
)