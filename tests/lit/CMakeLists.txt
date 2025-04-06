find_program(LLVM_LIT NAMES "lit" REQUIRED)
find_program(LLVM_FILE_CHECK NAMES "FileCheck" REQUIRED)
find_program(LLVM_LLC NAMES "llc" REQUIRED)
find_program(LLVM_CLANG NAMES "clang++" REQUIRED)

add_subdirectory(execution)

# this handles variables substitution
# like `@CMAKE_SOURCE_DIR@`
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in
        ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py.in
        @ONLY
)

# this handles generator expressions substitutions
# like `$<TARGET_FILE:cli>`
file(GENERATE
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
        INPUT ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py.in
        TARGET tests.lit
)

add_custom_target(
        tests.lit
        COMMAND ${LLVM_LIT} "${CMAKE_CURRENT_BINARY_DIR}" -v
        DEPENDS cli tests.lit.execution.library
)