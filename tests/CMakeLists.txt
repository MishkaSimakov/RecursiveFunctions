include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
        Tests
        testing_test.cpp
        feature/ProgramRunTests.cpp
)
target_include_directories(Tests PRIVATE feature)

target_link_libraries(
        Tests
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(Tests)