set(SOURCES
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

add_library(lexis_table_tools INTERFACE)
target_sources(lexis_table_tools INTERFACE ${SOURCES})

set(LEXIS_DFA_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/LexisDFA.inc)

tablegen(
        NAME lexis
        SOURCES ${SOURCES} compile_lexis_table.cpp
        OUTPUT ${LEXIS_DFA_OUTPUT}
)

target_compile_definitions(lexis_gen PRIVATE LEXIS_DFA_OUTPUT="${LEXIS_DFA_OUTPUT}")
target_include_directories(lexis_gen PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(lexis_gen PUBLIC fmt::fmt)