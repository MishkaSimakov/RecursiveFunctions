set(CoreModuleDirectories
    log preprocessor lexis syntax compilation execution
)

foreach(directory IN LISTS CoreModuleDirectories)
    file(GLOB_RECURSE Files
            "${directory}/*.cpp"
            "${directory}/*.h"
    )

    list(APPEND CoreFiles ${Files})
endforeach()

add_library(RecursiveFunctions ${CoreFiles}
        FunctionTraits.h)
target_include_directories(RecursiveFunctions PUBLIC ${PROJECT_SOURCE_DIR}/src)
