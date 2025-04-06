find_program(LLVM_FILE_CHECK NAMES "FileCheck" DOC "Path to llvm FileCheck tool" REQUIRED)
find_program(LLVM_LIT NAMES "lit" DOC "Path to llvm lit tool" REQUIRED)

get_target_property(TEA_COMPILER_DIR cli RUNTIME_OUTPUT_DIRECTORY)
set(TEA_COMPILER ${TEA_COMPILER_DIR}/cli)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in
        ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
)

add_custom_target(
        tests.lit
        COMMAND ${LLVM_LIT} "${CMAKE_CURRENT_BINARY_DIR}" -v
        DEPENDS cli
)