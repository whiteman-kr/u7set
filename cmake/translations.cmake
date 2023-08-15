cmake_minimum_required(VERSION 3.16)

function(add_translation target cmake_command output_directory)
    qt_add_lupdate(${target} TS_FILES translations/${TARGET}_uk.ts)
    add_dependencies(${target} ${target}_lupdate)
	
    qt_add_lrelease(${target}
	TS_FILES translations/${target}_uk.ts
        QM_FILES_OUTPUT_VARIABLE qm_files)
	
    add_custom_command(TARGET ${target} POST_BUILD COMMAND ${cmake_command} -E make_directory ${output_directory}/translations)
    add_custom_command(TARGET ${target} POST_BUILD COMMAND ${cmake_command} -E copy ${qm_files} ${output_directory}/translations)
endfunction()
