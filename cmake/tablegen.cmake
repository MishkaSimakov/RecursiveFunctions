function(tablegen)
    cmake_parse_arguments(TABLEGEN "" "NAME" "SOURCES;DEPENDENCIES;OUTPUT" ${ARGN})

    add_executable(${TABLEGEN_NAME}_gen ${TABLEGEN_SOURCES})

    add_custom_command(
            # run tablegen executable
            COMMAND ${TABLEGEN_NAME}_gen

            DEPENDS
            ${TABLEGEN_NAME}_gen
            ${TABLEGEN_SOURCES}
            ${TABLEGEN_DEPENDENCIES}

            OUTPUT ${TABLEGEN_OUTPUT}
            COMMENT "Running ${TABLEGEN_NAME} generator"
    )

    add_custom_target(${TABLEGEN_NAME}_run DEPENDS ${TABLEGEN_OUTPUT})

    add_library(${TABLEGEN_NAME} INTERFACE)
    add_dependencies(${TABLEGEN_NAME} ${TABLEGEN_NAME}_run)

    # extract directories from TABLEGEN_OUTPUT
    foreach (FILE IN LISTS TABLEGEN_OUTPUT)
        cmake_path(GET FILE PARENT_PATH FILE_DIRECTORY)
        list(APPEND TABLEGEN_OUTPUT_DIRS ${FILE_DIRECTORY})
    endforeach ()

    target_include_directories(${TABLEGEN_NAME} INTERFACE ${TABLEGEN_OUTPUT_DIRS})
endfunction()