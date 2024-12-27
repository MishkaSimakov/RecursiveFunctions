set(CoreModuleDirectories
        log optimizations compilation intermediate_representation assembly passes analysis filesystem
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
        syntax/grammar/Grammar.cpp
        syntax/lr/LRTableSerializer.cpp
        syntax/RecursiveFunctionsGrammar.cpp
)

# for lexis same as grammar
list(APPEND CoreFiles
        lexis/LexicalAnalyzer.cpp
        lexis/table/LexicalTableSerializer.cpp
)

add_library(RecursiveFunctions ${CoreFiles})
target_include_directories(RecursiveFunctions PUBLIC ${PROJECT_SOURCE_DIR}/src)
