set(SOURCES
        compile_grammar.cpp
        lr/LRTableBuilder.cpp
        lr/LRTableSerializer.cpp
        grammar/Grammar.cpp
        grammar/GrammarGenerator.cpp
)

tablegen(
        NAME grammar
        SOURCES ${SOURCES}
        DEPENDENCIES grammar.txt
        OUTPUT ${CMAKE_SOURCE_DIR}/files/grammar/grammar.lr
)

target_include_directories(grammar_tablegen PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_compile_definitions(grammar_tablegen PUBLIC COMPILING_GRAMMAR)
target_link_libraries(grammar_tablegen PUBLIC fmt::fmt)