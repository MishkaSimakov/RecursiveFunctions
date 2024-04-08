cmake_minimum_required(VERSION 3.28)
project(RecursiveFunctions)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "-fsanitize=address")

add_executable(RecursiveFunctions main.cpp
        src/preprocessor/Preprocessor.h
        src/syntax/buffalo/SyntaxTreeBuilder.h
        src/lexis/LexicalAnalyzer.h
        src/log/Logger.h
        src/syntax/buffalo/SyntaxConsumers.h
        src/syntax/RecursiveFunctionsSyntax.h
        src/syntax/buffalo/SyntaxNode.h
        src/syntax/buffalo/SyntaxNode.h
        src/syntax/AdditionSyntax.h
        src/compilation/instructions.h
        src/compilation/IdentifiersEnumerator.h)

target_include_directories(RecursiveFunctions PUBLIC src)