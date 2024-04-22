add_library(RecursiveFunctions INTERFACE
        compilation/CompileTreeBuilder.h
        compilation/CompileTreeNodes.h
        compilation/bytecode/BytecodeCompiler.h
        compilation/bytecode/CompileFunctionDefinition.cpp
        compilation/bytecode/CompileProgram.cpp
        compilation/bytecode/CompileFunctionCall.cpp
        compilation/bytecode/CompileVariable.cpp
        compilation/bytecode/CompileRecursiveFunctionDefinition.cpp
        compilation/bytecode/CompileInternalFunctionDefinition.cpp
)

target_include_directories(RecursiveFunctions SYSTEM INTERFACE ../src)