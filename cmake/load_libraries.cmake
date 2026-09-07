# This cmake file builds and finds all external libraries that are used in this project.

# -- llvm --
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

if (NOT ${LLVM_VERSION_MAJOR} EQUAL 23)
    message(WARNING "Support for LLVM ${LLVM_VERSION_MAJOR} is not guaranteed. Use LLVM 23.")
endif ()

llvm_map_components_to_libnames(llvm_libs support core irreader linker
        AArch64AsmParser
        AArch64CodeGen
        AArch64Desc
        AArch64Disassembler
        AArch64Info
        AArch64Utils
        CodeGen
        AsmParser
        AsmPrinter
        Target
        TargetParser
)
# -- llvm end --

# -- fmt --
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
add_subdirectory(lib/fmt EXCLUDE_FROM_ALL)
# -- fmt end --

# -- argparse --
add_subdirectory(lib/argparse EXCLUDE_FROM_ALL)
# -- argparse end --

# -- google test --
add_subdirectory(lib/googletest EXCLUDE_FROM_ALL)
# -- google test end --