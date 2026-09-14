set(SOURCES
        lr/LRTableBuilder.cpp
        lr/LRTableSerializer.cpp
        grammar/Grammar.cpp
        grammar/GrammarGenerator.cpp
)

add_library(grammar_table_tools INTERFACE)
target_sources(grammar_table_tools INTERFACE ${SOURCES})

set(GRAMMAR_TABLEGEN_TEXT_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/grammar.txt")
set(GRAMMAR_TABLEGEN_BUILDERS_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/SyntaxASTBuildersRegistry.inc")
set(GRAMMAR_TABLEGEN_LR_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/SyntaxLR.inc")

tablegen(
        NAME grammar
        SOURCES ${SOURCES} compile_grammar.cpp
        DEPENDENCIES ${GRAMMAR_TABLEGEN_TEXT_INPUT} lexis_table_tools
        OUTPUT ${GRAMMAR_TABLEGEN_LR_OUTPUT} ${GRAMMAR_TABLEGEN_BUILDERS_OUTPUT}
)

target_link_libraries(grammar_gen PUBLIC fmt::fmt)
target_include_directories(grammar_gen PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_compile_definitions(grammar_gen PRIVATE
        GRAMMAR_TABLEGEN_TEXT_INPUT="${GRAMMAR_TABLEGEN_TEXT_INPUT}"
        GRAMMAR_TABLEGEN_BUILDERS_OUTPUT="${GRAMMAR_TABLEGEN_BUILDERS_OUTPUT}"
        GRAMMAR_TABLEGEN_LR_OUTPUT="${GRAMMAR_TABLEGEN_LR_OUTPUT}"
)