cmake_minimum_required(VERSION 3.16)

project(AdsBridge 
	LANGUAGES CXX 
	VERSION ${U7SET_MAJOR_VERSION}.${U7SET_MINOR_VERSION}.${U7SET_PATCH_VERSION}
	DESCRIPTION "AppDataService Bridge"
	HOMEPAGE_URL "radiy.com")

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core)

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

target_link_libraries(AdsBridge PRIVATE Qt6::Core)
target_link_libraries(AdsBridge PRIVATE AppSignalLib OnlineLib ClientLib)

target_include_directories(AdsBridge PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(AdsBridge INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(AdsBridge CODE_COVERAGE)

# --
#
add_subdirectory(examples)
#add_subdirectory(tests)
