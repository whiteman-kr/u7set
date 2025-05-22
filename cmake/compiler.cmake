function(set_compiler_standard TARGET)
	target_compile_features(${TARGET} PUBLIC cxx_std_20)
	
	set_target_properties(${TARGET} PROPERTIES
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
	)
endfunction()

function(set_warning_level_1 TARGET)
	# Set warning level 1 for MSVC, override default
	#
	if(MSVC)
	  # Remove any inherited /W flags (e.g., /W3, /W4, /Wall)
	  #
	  get_target_property(old_opts ${TARGET} COMPILE_OPTIONS)
	  if(old_opts)
		list(FILTER old_opts EXCLUDE REGEX "^/W[0-9]")
		set_target_properties(${TARGET} PROPERTIES COMPILE_OPTIONS "${old_opts}")
	  endif()

	  # Set minimal warning level
	  #
	  target_compile_options(${TARGET} PRIVATE /W1)
	else()
	  # Suppress all warnings (GCC/Clang)
	  #
	  target_compile_options(${TARGET} PRIVATE -w)
	endif()
endfunction()