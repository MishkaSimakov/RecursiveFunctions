set(SOURCES
        lr/LRTableBuilder.cpp
        lr/LRTableSerializer.cpp
        grammar/Grammar.cpp
        grammar/GrammarGenerator.cpp
)

add_library(grammar_table_tools INTERFACE)
target_sources(grammar_table_tools INTERFACE ${SOURCES})

set(GRAMMAR_TABLEGEN_TEXT_INPUT "${CMAKE_SOURCE_DIR}/src/syntax/grammar.txt")
set(GRAMMAR_TABLEGEN_BUILDERS_OUTPUT "${CMAKE_SOURCE_DIR}/src/syntax/BuildersRegistry.h")

# We add lexis_table_tools as dependency, because when tokens and their representation changes
# we must update grammar files too.
tablegen(
        NAME grammar
        SOURCES ${SOURCES} compile_grammar.cpp
        DEPENDENCIES ${GRAMMAR_TABLEGEN_TEXT_INPUT} lexis_table_tools
        OUTPUT ${TEALANG_FILES_DIRECTORY}/grammar/grammar.lr ${GRAMMAR_TABLEGEN_BUILDERS_OUTPUT}
)

target_include_directories(grammar_tablegen PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(grammar_tablegen PUBLIC fmt::fmt)
target_compile_definitions(grammar_tablegen PRIVATE
        GRAMMAR_TABLEGEN_TEXT_INPUT="${GRAMMAR_TABLEGEN_TEXT_INPUT}"
        GRAMMAR_TABLEGEN_BUILDERS_OUTPUT="${GRAMMAR_TABLEGEN_BUILDERS_OUTPUT}"
)