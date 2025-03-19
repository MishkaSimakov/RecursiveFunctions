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

tablegen(
        NAME lexis
        SOURCES ${SOURCES} compile_lexis_table.cpp
        OUTPUT ${CMAKE_SOURCE_DIR}/files/lexis/lexis.lx
)

target_include_directories(lexis_tablegen PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(lexis_tablegen PUBLIC fmt::fmt)