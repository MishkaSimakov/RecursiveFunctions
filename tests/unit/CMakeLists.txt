include(GoogleTest)

file(GLOB_RECURSE TESTS_SOURCES "*.cpp")

add_executable(tests.unit ${TESTS_SOURCES})
target_link_libraries(tests.unit TeaLang lexis_table_tools GTest::gtest_main)
gtest_discover_tests(tests.unit)