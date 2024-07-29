cmake_minimum_required(VERSION 3.16)

project(UdpRtr LANGUAGES CXX)

include(../../cmake/vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Network)
qt_standard_project_setup()

set(TARGET UdpRtr)

add_executable(${TARGET}
	CaptureDevice.cpp
	CaptureDevice.h
	CircularLogger.cpp
	CircularLogger.h
	IpHeaders.cpp
	IpHeaders.h
	UdpRetranslatorApp.cpp
	UdpRetranslatorApp.h
	UdpRtrMain.cpp
)

# Add LTO for Release and Windows only. 
# It is diasbale for Unix to speed up CI bulid.
#
if(CMAKE_BUILD_TYPE STREQUAL "Release" AND MSVC)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Where to put binaries.
#
if (DEFINED BIN_OUT_DIR)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_OUT_DIR})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_OUT_DIR})
else()
	string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_LOWERCASE)

	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${BUILD_TYPE_LOWERCASE})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${BUILD_TYPE_LOWERCASE})
endif()
                                           
target_precompile_headers(${TARGET} PRIVATE
	# C includes, must be before c++ includes
	#
	<cassert>

	# C++ includes
	#
	<condition_variable>
	<iostream>
	<limits>
	<map>
	<memory>
	<mutex>
	<optional>
	<queue>
	<set>
	<thread>
	<type_traits>
	<vector>

	# Windows includes

	<tchar.h>
	<winsock2.h>		# - don't change order of this includes!
	<windows.h>			# /

	# Qt includes

	<QCoreApplication>
	<QDateTime>
	<QDir>
	<QFile>
	<QFileInfo>
	<QStandardPaths>
	<QTextStream>
	<QThread>
)

include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../../UtilsLib)
include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../../CommonLib)
include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/npcap/include)

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Network)
#target_link_libraries(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/npcap/Lib/x64/Packet.lib)
target_link_libraries(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/npcap/Lib/x64/wpcap.lib)


# Link Visaul Leak Detector

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()
