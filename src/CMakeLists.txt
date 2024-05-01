add_library(RecursiveFunctions
        compilation/bytecode/CompileFunctionDefinition.cpp
        compilation/bytecode/CompileProgram.cpp
        compilation/bytecode/CompileFunctionCall.cpp
        compilation/bytecode/CompileVariable.cpp
        compilation/bytecode/CompileRecursiveFunctionDefinition.cpp
        compilation/bytecode/CompileInternalFunctionDefinition.cpp
        compilation/bytecode/AssembleInstructions.cpp
        compilation/bytecode/CompileRecursionParameter.cpp
        compilation/bytecode/CompileArgminCall.cpp
        compilation/bytecode/CompileConstant.cpp
        compilation/bytecode/CompileAsterisk.cpp
        compilation/bytecode/CompileSelfCall.cpp
        preprocessor/Source.h
        preprocessor/FileSource.h
        preprocessor/TextSource.h
        cli/Main.h
        preprocessor/Exceptions.h
        lexis/Exceptions.h
)

target_include_directories(RecursiveFunctions PUBLIC "${PROJECT_SOURCE_DIR}/src")