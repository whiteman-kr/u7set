function(set_warnings)

	if(MSVC)
		add_compile_options(/W4)
		add_compile_options(/wd4201) # nonstandard extension used: nameless struct/union
		add_compile_options(/wd4458) # declaration of 'identifier' hides class member
	endif()

endfunction()

