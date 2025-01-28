function(copy_libpq_dll TARGET)
	#
	# Copy PostgreSQL DLLs (libpq.dll and its' dependencies)
	# These dlls are not copied by default.
	#
	if(WIN32)
		target_link_libraries(${TARGET} PUBLIC PostgreSQL::PostgreSQL)

		get_target_property(PG_LOCATION PostgreSQL::PostgreSQL IMPORTED_LOCATION)
		get_target_property(PG_DLL PostgreSQL::PostgreSQL IMPORTED_LOCATION_DEBUG)  
		# Here libpq.lib is shown, it is a library to use this dll.
		# This is just for information, not used in the script.
		# Actual dll is near in ../bin folder.
		#
		message("PostgreSQL DLL location: ${PG_LOCATION} | Debug: ${PG_DLL}")

		# Usually, vcpkg installs the DLL in:
		set(POSTGRESQL_BIN_DIR "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin")
		set(POSTGRESQL_DLLS
			"${POSTGRESQL_BIN_DIR}/libpq.dll"
			"${POSTGRESQL_BIN_DIR}/libssl-3-x64.dll"
			"${POSTGRESQL_BIN_DIR}/libcrypto-3-x64.dll"
		)

		# Copy them after building your target:
		foreach(DLL ${POSTGRESQL_DLLS})
			if(EXISTS "${DLL}")
				add_custom_command(TARGET ${TARGET} POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy_if_different
						"${DLL}"
						"$<TARGET_FILE_DIR:${TARGET}>"
				)
			else()
				message(WARNING "PostgreSQL DLL not found: ${DLL}")
			endif()
		endforeach()
	endif()
endfunction()