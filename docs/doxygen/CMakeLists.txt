
find_package(Doxygen)

add_custom_target(docs)

if (DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
    configure_file(Doxyfile.in Doxyfile @ONLY)
    add_custom_target(doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        COMMENT "Generating Doxygen Documentation"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    add_dependencies(docs doxygen)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/ DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/aktualizr OPTIONAL)
else(DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
    message(WARNING "doxygen + dot not found, skipping")
endif(DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
