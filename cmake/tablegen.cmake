function(add_tablegen_cmake_directory)
    add_custom_target(
            prepare_tablegen_cmake_directory
            COMMAND
            ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}"
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            ${CMAKE_SOURCE_DIR}
            -DBUILD_ONLY_TABLEGEN=ON
            WORKING_DIRECTORY ${TABLEGEN_CMAKE_DIR}
            COMMENT "Running CMake for tablegen targets"
    )
    file(MAKE_DIRECTORY ${TABLEGEN_CMAKE_DIR})
endfunction()

function(tablegen)
    cmake_parse_arguments(TABLEGEN "" "NAME" "SOURCES;DEPENDENCIES;OUTPUT" ${ARGN})

    add_executable(${TABLEGEN_NAME}_tablegen ${TABLEGEN_SOURCES})
    set_target_properties(${TABLEGEN_NAME}_tablegen PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${TABLEGEN_CMAKE_DIR}/${TABLEGEN_NAME}
    )

    add_custom_target(${TABLEGEN_NAME}_tablegen_dsl DEPENDS ${TABLEGEN_DSL_DEPENDENCIES})
    add_dependencies(${TABLEGEN_NAME}_tablegen ${TABLEGEN_NAME}_tablegen_dsl)

    add_custom_command(
            # build cmake target for current tablegen
            COMMAND ${CMAKE_COMMAND} --build . --target ${TABLEGEN_NAME}_tablegen

            # run tablegen executable
            COMMAND ${TABLEGEN_CMAKE_DIR}/${TABLEGEN_NAME}/${TABLEGEN_NAME}_tablegen

            DEPENDS
            prepare_tablegen_cmake_directory
            ${TABLEGEN_SOURCES}
            ${TABLEGEN_DEPENDENCIES}

            OUTPUT ${TABLEGEN_OUTPUT}
            WORKING_DIRECTORY ${TABLEGEN_CMAKE_DIR}
            COMMENT "Running ${TABLEGEN_NAME} tablegen"
    )
    add_custom_target(run_${TABLEGEN_NAME}_tablegen DEPENDS ${TABLEGEN_OUTPUT})
endfunction()