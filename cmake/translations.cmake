function(add_translation target output_directory)
    
    set(TS_FILES translations/${target}_uk.ts translations/${target}_bg.ts)
    
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/translations/${target}_ru.ts)
        list(APPEND TS_FILES translations/${target}_ru.ts)
    endif()
    
    qt_add_lupdate(${target} TS_FILES ${TS_FILES})

    if(UPDATE_TRANSLATIONS)
        add_dependencies(${target} ${target}_lupdate)
    endif()
	
    qt_add_lrelease(${target}
	    TS_FILES ${TS_FILES}
        QM_FILES_OUTPUT_VARIABLE qm_files)
	
    add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${output_directory}/translations)
    add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${qm_files} ${output_directory}/translations)
endfunction()
