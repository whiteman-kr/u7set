cmake_minimum_required(VERSION 3.16)

function(add_translation)
    qt_add_lupdate(${TARGET} TS_FILES languages/${TARGET}_uk.ts)
    add_dependencies(${TARGET} ${TARGET}_lupdate)
	
    qt_add_lrelease(${TARGET}
        TS_FILES languages/${TARGET}_uk.ts
        QM_FILES_OUTPUT_VARIABLE qm_files)
	
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/languages)
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${qm_files} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/languages)
endfunction()
