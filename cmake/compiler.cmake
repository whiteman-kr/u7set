function(set_compiler_standard TARGET)
	
	target_compile_features(${TARGET} PUBLIC cxx_std_20)
	
	set_target_properties(${TARGET} PROPERTIES
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
	)

endfunction()