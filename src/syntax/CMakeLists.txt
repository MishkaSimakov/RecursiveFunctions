add_executable(CompileGrammar
        compile_grammar.cpp
        RecursiveFunctionsGrammar.cpp
        lr/LRTableBuilder.cpp
        lr/LRTableSerializer.cpp
        grammar/Grammar.cpp
)
target_include_directories(CompileGrammar PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_compile_definitions(CompileGrammar PUBLIC COMPILING_GRAMMAR)