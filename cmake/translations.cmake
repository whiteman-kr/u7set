cmake_minimum_required(VERSION 3.28)

function(add_translation target output_directory)
    qt_add_lupdate(${target} TS_FILES translations/${TARGET}_uk.ts translations/${TARGET}_bg.ts)

    if(UPDATE_TRANSLATIONS)
        add_dependencies(${target} ${target}_lupdate)
    endif()
	
    qt_add_lrelease(${target}
	TS_FILES translations/${target}_uk.ts
	TS_FILES translations/${target}_bg.ts
        QM_FILES_OUTPUT_VARIABLE qm_files)
	
    add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${output_directory}/translations)
    add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${qm_files} ${output_directory}/translations)
endfunction()
