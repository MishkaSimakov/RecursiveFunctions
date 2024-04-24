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
)

target_link_libraries(
        Tests
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(Tests)

add_custom_command(
        TARGET Tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/programs
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/tests/feature/programs/*
        ${CMAKE_CURRENT_BINARY_DIR}/programs)