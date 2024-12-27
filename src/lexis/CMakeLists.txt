add_executable(CompileLexisTable
        compile_lexis_table.cpp
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
target_include_directories(CompileLexisTable PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(CompileLexisTable PUBLIC fmt::fmt)