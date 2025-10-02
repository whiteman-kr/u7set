cmake_minimum_required(VERSION 3.16)

project(CommonLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 REQUIRED COMPONENTS Core)

set(TARGET CommonLib)

qt_add_library(${TARGET} STATIC
  ./include/CommonLib/AfbParamValue.h
  ./include/CommonLib/ConstStrings.h
  ./include/CommonLib/DebugInstCounter.h
  ./include/CommonLib/Factory.h
  ./include/CommonLib/Hash.h
  ./include/CommonLib/HashedVector.h
  ./include/CommonLib/HostAddressPort.h
  ./include/CommonLib/PropertyObject.h
  ./include/CommonLib/Times.h
  ./include/CommonLib/Types.h
  ./include/CommonLib/expected.hpp
  ./include/CommonLib/u7_vld.h
  AfbParamValue.cpp
  PropertyObject.cpp
  Times.cpp
  Types.cpp
  u7_vld.cpp
)


target_precompile_headers(${TARGET} PRIVATE
	<QReadWriteLock>
	<QReadLocker>
	<QWriteLocker>
)

#target_precompile_headers(${TARGET} PUBLIC
#	"Types.h"
#	"Hash.h"
#	"PropertyObject.h"
#	"AfbParamValue.h"
#)

target_link_libraries(${TARGET} PUBLIC Qt6::Core)

target_compile_definitions(${TARGET} PRIVATE COMMON_LIB_DOMAIN)
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	
	set(VLD_PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}")
	string(REPLACE "/" "\\\\"
	    VLD_PROJECT_ROOT_PATH_ESCAPED "${VLD_PROJECT_ROOT_PATH}")

	add_compile_definitions(VLD_PROJECT_ROOT_PATH=L"${VLD_PROJECT_ROOT_PATH_ESCAPED}")

	target_link_libraries(${TARGET} PUBLIC "C:/Program Files (x86)/Visual Leak Detector/lib/Win64/vld.lib")
endif()

add_code_coverage_options(${TARGET} CODE_COVERAGE)
