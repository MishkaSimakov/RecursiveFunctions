set_target_properties(cli_installation PROPERTIES OUTPUT_NAME tlang)

install(TARGETS cli_installation)
install(DIRECTORY ${TEALANG_FILES_DIRECTORY} DESTINATION share/tlang
        PATTERN "*.gitkeep" EXCLUDE
)