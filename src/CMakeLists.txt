set(CoreModuleDirectories
        log filesystem ast sources errors compilation
)

foreach (directory IN LISTS CoreModuleDirectories)
    file(GLOB_RECURSE Files
            "${directory}/*.cpp"
            "${directory}/*.h"
    )

    list(APPEND CoreFiles ${Files})
endforeach ()

# we add syntax directory separately because some files are only needed for grammar generation
list(APPEND CoreFiles
        syntax/lr/LRParser.cpp
        syntax/lr/LRTableSerializer.cpp
)

# for lexis same as grammar
list(APPEND CoreFiles
        sources/SourceManager.cpp
        lexis/LexicalAnalyzer.cpp
        lexis/table/LexicalTableSerializer.cpp
)

add_library(TeaLang ${CoreFiles})
target_include_directories(TeaLang PUBLIC .)
target_link_libraries(TeaLang
        PUBLIC fmt::fmt
        PUBLIC argparse::argparse
        PRIVATE ${llvm_libs}
)
target_include_directories(TeaLang PUBLIC ${LLVM_INCLUDE_DIRS})

# TeaLang requires grammar and lexis tables to work
add_dependencies(TeaLang run_grammar_tablegen run_lexis_tablegen)
