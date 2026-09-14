install(TARGETS tlang)

install(
        DIRECTORY ${CMAKE_BINARY_DIR}/lib/tlang/${CMAKE_PROJECT_VERSION}/lib
        DESTINATION                   lib/tlang/${CMAKE_PROJECT_VERSION}
)
install(
        DIRECTORY ${CMAKE_BINARY_DIR}/lib/tlang/${CMAKE_PROJECT_VERSION}/include
        DESTINATION                   lib/tlang/${CMAKE_PROJECT_VERSION}
)