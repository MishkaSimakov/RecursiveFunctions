add_library(tests.lit.execution.library STATIC library.cpp)
set_target_properties(
        tests.lit.execution.library PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        LIBRARY_OUTPUT_NAME "library"
)