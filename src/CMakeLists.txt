set(CoreModuleDirectories
        log ast sources errors compilation interpretation
)

foreach (directory IN LISTS CoreModuleDirectories)
    file(GLOB_RECURSE Files
            "${directory}/*.cpp"
            "${directory}/*.h"
    )

    list(APPEND CoreFiles ${Files})
endforeach ()

# temporarily remove compilation/ir folder
list(FILTER CoreFiles EXCLUDE REGEX "/compilation\/ir/")
list(FILTER CoreFiles EXCLUDE REGEX "/compilation\/semantics/")

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
target_include_directories(TeaLang PUBLIC ${PROJECT_SOURCE_DIR}/src)
