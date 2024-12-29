set(CoreModuleDirectories
        log optimizations intermediate_representation assembly passes analysis filesystem ast sources errors
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

add_library(RecursiveFunctions ${CoreFiles})
target_include_directories(RecursiveFunctions PUBLIC ${PROJECT_SOURCE_DIR}/src)
