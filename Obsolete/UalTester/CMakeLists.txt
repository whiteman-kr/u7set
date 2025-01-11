cmake_minimum_required(VERSION 3.25)

project(UalTester LANGUAGES CXX)

include(../../cmake/vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Qml)
qt_standard_project_setup()

# Add icon
#
if (MSVC)
	set (WINDOWS_RESOURCES ./WinResources/WinResources.rc)
endif (MSVC)

set(TARGET UalTester)

add_executable(${TARGET}
	CmdLineParam.cpp
	CmdLineParam.h
	main.cpp
	SignalBase.cpp
	SignalBase.h
	SignalStateSocket.cpp
	SignalStateSocket.h
	TestFile.cpp
	TestFile.h
	TuningSocket.cpp
	TuningSocket.h
	TuningSourceBase.cpp
	TuningSourceBase.h
	UalTester.cpp
	UalTester.h

	Resources.qrc
	${WINDOWS_RESOURCES}
)

target_precompile_headers(${TARGET} PRIVATE
	# C/C++ includes
	#
	<cassert>
	<cstdint>
	<algorithm>
	<atomic>
	<functional>
	<iostream>
	<limits>
	<list>
	<map>
	<memory>
	<vector>
	<set>
	<type_traits>

	# Qt includes
	#
	#<QtNetwork>
	#<QtQml>
)

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Network Qt6::Qml)
target_link_libraries(${TARGET} PRIVATE CommonLib ProtobufEx UtilsLib AppSignalLib ServiceLib OnlineLib  HardwareLib)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	 target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()

