cmake_minimum_required(VERSION 3.16)

project(AdsBridge 
	LANGUAGES CXX 
	VERSION ${U7SET_MAJOR_VERSION}.${U7SET_MINOR_VERSION}.${U7SET_PATCH_VERSION}
	DESCRIPTION "AppDataService Bridge"
	HOMEPAGE_URL "radiy.com")

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Network)

#
# AdsBridge
#
add_library(AdsBridge SHARED 
	include/AdsBridge/AdsBridge.def
	include/AdsBridge/AdsBridge.h
	include/AdsBridge/Common.h
	include/AdsBridge/SelfTest.h
	src/AdsBridge.cpp
	src/AdsBridgeFacade.cpp
	src/AdsBridgeLogFile.cpp
	src/AdsBridgeLogFile.h
 )

target_precompile_headers(AdsBridge PRIVATE
	"../../CommonLib/Hash.h"
	"../../CommonLib/Types.h"

	"../../AppSignalLib/AppSignalParam.h"
	"../../AppSignalLib/AppSignalState.h"
)

target_link_libraries(AdsBridge PRIVATE Qt6::Network)
target_link_libraries(AdsBridge PRIVATE AppSignalLib OnlineLib ClientLib)

target_include_directories(AdsBridge PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(AdsBridge INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(AdsBridge CODE_COVERAGE)

add_custom_command(
  TARGET AdsBridge
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dev/include"
)

# if this is a Windos build, copy the AdsBridge.lib to the output directory
if(WIN32)
	add_custom_command(
		TARGET AdsBridge
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dev/lib
	)
	add_custom_command(
		TARGET AdsBridge
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different AdsBridge.lib ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dev/lib/AdsBridge.lib
	)
	add_custom_command(
		TARGET AdsBridge
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/MsvcGenLib.txt ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dev/lib/
	)
endif()

# Generate API documentation
#

# Find for Doxygen here as add_subdirectory works with CMake 3.27+ only
#
find_package(Doxygen)

if(DOXYGEN_FOUND)
	message(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE}")

	add_custom_target(BridgeApiDoc ALL
		COMMAND ${CMAKE_COMMAND} -E env DOXYGEN_OUT_DIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dev/ ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/doc/ApiDoxygen.cfg
		COMMENT "Generating BridgeAPI documentation with Doxygen"
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/doc
		VERBATIM)

	add_dependencies(AdsBridge BridgeApiDoc)
endif()

# --
#
add_subdirectory(examples/cpp)
#add_subdirectory(tests)
