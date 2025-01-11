cmake_minimum_required(VERSION 3.28)

project(ScriptHelp)

find_package(Doxygen REQUIRED)

set(TARGET "u7ScriptHelp")

 add_custom_target(${TARGET} ALL
        COMMAND ${CMAKE_COMMAND} -E env DOXYGEN_OUT_DIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripthelp ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/ScriptHelp.cfg
        COMMENT "Generating API documentation with Doxygen"
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        VERBATIM)
