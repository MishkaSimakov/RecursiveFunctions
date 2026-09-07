# Targets for standard library building and deployment

file(GLOB_RECURSE STD_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.h")

add_library(std STATIC ${STD_SOURCES})
set_target_properties(std PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${TEALANG_FILES_DIRECTORY}/std/lib
)

file(GLOB_RECURSE STD_HEADERS CONFIGURE_DEPENDS
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include
        "include/*.tea" "include/*.team"
)

set(STD_COPIED_HEADERS "")
foreach (header ${STD_HEADERS})
    set(copied_header ${TEALANG_FILES_DIRECTORY}/std/include/${header})

    add_custom_command(
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/include/${header} ${copied_header}

            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/${header}

            OUTPUT ${copied_header}
            COMMENT "Copying standard library header ${header}"
    )

    list(APPEND STD_COPIED_HEADERS ${copied_header})
endforeach ()

add_custom_target(std_headers ALL DEPENDS ${STD_COPIED_HEADERS})
