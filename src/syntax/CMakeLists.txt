set(SOURCES
        lr/LRTableBuilder.cpp
        lr/LRTableSerializer.cpp
        grammar/Grammar.cpp
        grammar/GrammarGenerator.cpp
)

add_library(grammar_table_tools INTERFACE)
target_sources(grammar_table_tools INTERFACE ${SOURCES})

# We add lexis_table_tools as dependency, because when tokens and their representation changes
# we must update grammar files too.
tablegen(
        NAME grammar
        SOURCES ${SOURCES} compile_grammar.cpp
        DEPENDENCIES grammar.txt lexis_table_tools
        OUTPUT ${CMAKE_SOURCE_DIR}/files/grammar/grammar.lr ${CMAKE_SOURCE_DIR}/src/syntax/BuildersRegistry.h
)

target_include_directories(grammar_tablegen PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(grammar_tablegen PUBLIC fmt::fmt)