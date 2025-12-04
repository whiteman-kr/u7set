cmake_minimum_required(VERSION 3.21)

project(
	AdsBridge 
	LANGUAGES CXX
	VERSION ${U7SET_MAJOR_VERSION}.${U7SET_MINOR_VERSION}.${U7SET_PATCH_VERSION}
	DESCRIPTION "AppDataService Bridge"
	HOMEPAGE_URL "radiy.com"
)

find_package(pugixml CONFIG REQUIRED)

#
# AdsBridge
#
set(TARGET AdsBridge)

add_library(${TARGET} SHARED)
add_library(Radiy::${TARGET} ALIAS ${TARGET})

target_sources(
	${TARGET}
	PRIVATE
		./include/AdsBridge/AdsBridge.h
		./include/AdsBridge/Common.h
		
		./src/AdsbAppSignalManager.cpp
		./src/AdsbAppSignalManager.h
		./src/AdsBridge.cpp
		./src/AdsBridgeFacade.cpp
		./src/AdsBridgeLogFile.cpp
		./src/AdsBridgeLogFile.h
		./src/AdsBridgeResources.cpp
		./src/AdsBridgeResources.h
 )

set_compiler_standard(${TARGET})
add_code_coverage_options(${TARGET} CODE_COVERAGE)

target_precompile_headers(
	${TARGET} 
	PRIVATE
		<CommonStdLib/HashStd.h>

		<Network.pb.h>
)

# Include directories
#
target_include_directories(
	${TARGET}
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

# Hide all symbols except explicitly exported ones
#
target_compile_definitions(${TARGET} PRIVATE ADSBRIDGE_EXPORT)

if(MSVC)
	set_target_properties(${TARGET} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS OFF)
else()
	target_compile_options(${TARGET} PRIVATE -fvisibility=hidden)
endif()

target_link_libraries(${TARGET} PRIVATE pugixml)
target_link_libraries(${TARGET} PRIVATE AdsConnectionLib)

#
# INSTALL, based on video tutorial https://www.youtube.com/watch?v=OMx3cZj_hoo&ab_channel=Codeforyourself
#
install(
	TARGETS ${TARGET}
	EXPORT ${TARGET}_export
	RUNTIME     DESTINATION bin             # .dll (Windows)
	LIBRARY     DESTINATION lib             # .so/.dylib (Linux/macOS)
	ARCHIVE     DESTINATION lib             # .lib/.a
	INCLUDES    DESTINATION include
)

# Copy every non-system DLL that AdsBridge.dll depends on
# (this is recursive: it follows links to AppSignalLib, OnlineLib, ...)
#
install(
	FILES
	$<TARGET_RUNTIME_DLLS:AdsBridge>
	DESTINATION bin
)

install(
	DIRECTORY include/${TARGET}
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
	EXPORT ${TARGET}_export
	FILE ${TARGET}_export.cmake
	NAMESPACE Radiy::
	DESTINATION share/cmake/${TARGET}
)

# Generate and copy configuration files
#
include(CMakePackageConfigHelpers)

configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/${TARGET}Config.cmake"
	INSTALL_DESTINATION "share/cmake/${TARGET}"
)

write_basic_package_version_file(
	${TARGET}ConfigVersion.cmake
	VERSION ${PACKAGE_VERSION}
	COMPATIBILITY AnyNewerVersion
)

install(
	FILES
		${CMAKE_CURRENT_BINARY_DIR}/${TARGET}Config.cmake
		${CMAKE_CURRENT_BINARY_DIR}/${TARGET}ConfigVersion.cmake
	DESTINATION share/cmake/${TARGET}
)

install(
	FILES
		"${CMAKE_CURRENT_SOURCE_DIR}/README.md"
	DESTINATION ./
)

install(
	DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dev/BridgeApiHelp/
	DESTINATION "share/doc/AdsBridge" # ${CMAKE_INSTALL_DOCDIR} is share/doc/u7set, so use manual setting to AdsBridge
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/examples
	DESTINATION "./"
)

#
# Generating output
#
add_custom_command(
  TARGET ${TARGET}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dev/include"
)

# Generate API documentation
#

# Find for Doxygen here as add_subdirectory works with CMake 3.27+ only
#
find_package(Doxygen)

if(DOXYGEN_FOUND)
	message(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE}")

	add_custom_target(
		BridgeApiDoc ALL
		COMMAND ${CMAKE_COMMAND} -E env DOXYGEN_OUT_DIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dev/ ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/doc/ApiDoxygen.cfg
		COMMENT "Generating BridgeAPI documentation with Doxygen"
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/doc
		VERBATIM
	)

	add_dependencies(${TARGET} BridgeApiDoc)
endif()

add_subdirectory(tests)
