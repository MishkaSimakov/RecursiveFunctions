function(tablegen)
    cmake_parse_arguments(TABLEGEN "" "NAME" "SOURCES;DEPENDENCIES;OUTPUT" ${ARGN})

    add_executable(${TABLEGEN_NAME}_tablegen ${TABLEGEN_SOURCES})
    set_target_properties(${TABLEGEN_NAME}_tablegen PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tablegen/${TABLEGEN_NAME}
    )

    add_custom_target(${TABLEGEN_NAME}_tablegen_dependencies DEPENDS ${TABLEGEN_DEPENDENCIES})
    add_dependencies(${TABLEGEN_NAME}_tablegen ${TABLEGEN_NAME}_tablegen_dependencies)

    add_custom_command(
            # run tablegen executable
            COMMAND ${CMAKE_BINARY_DIR}/tablegen/${TABLEGEN_NAME}/${TABLEGEN_NAME}_tablegen

            DEPENDS
            ${TABLEGEN_NAME}_tablegen
            ${TABLEGEN_SOURCES}
            ${TABLEGEN_DEPENDENCIES}

            OUTPUT ${TABLEGEN_OUTPUT}
            COMMENT "Running ${TABLEGEN_NAME} tablegen"
    )
    add_custom_target(run_${TABLEGEN_NAME}_tablegen DEPENDS ${TABLEGEN_OUTPUT})
endfunction()