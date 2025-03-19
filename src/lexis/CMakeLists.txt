# Some classes from Lexis are not required for main compiler build.
# Though they may be required for other builds (for example: lexis table compilation).
# All this tools are collected in this library.
add_library(LexisTools INTERFACE)

target_sources(LexisTools PUBLIC
        table/LexicalAutomatonGenerator.cpp
        table/LexicalTableSerializer.cpp
        regex/CustomRegex.cpp
        regex/RegexPrinter.cpp
        regex/RegexParser.cpp
        automata/FiniteAutomata.cpp
        automata/NonDeterministicFiniteAutomata.cpp
        automata/parts/DeterministicToMinimal.cpp
        automata/parts/NonDeterministicToDeterministic.cpp
)

add_executable(CompileLexisTable compile_lexis_table.cpp)
target_include_directories(CompileLexisTable PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(CompileLexisTable PUBLIC fmt::fmt LexisTools)