cmake_minimum_required(VERSION 3.16)

project(RvModelSimTester LANGUAGES CXX)

include(../../cmake/vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Widgets Qml Xml)
qt_standard_project_setup()

set(TARGET RvModelSimTester)

set (TC_SOURCES
	main.cpp
    RWToolBox.cpp
    RWToolBox.h
	DialogSettings.cpp
	DialogSettings.h
	SimControlModule.cpp
	SimControlModule.h
	LogModule.h
	LogModule.cpp
	MainWindow.cpp
	MainWindow.h
)

set (TC_RESOURCES
#	Resources.qrc
#	../Etc/Logo/RadiyLogo.qrc
)

set (TC_FORMS
	
)

add_executable(${TARGET} WIN32
	${TC_FORMS}
	${TC_SOURCES}
	${TC_RESOURCES}
	${WINDOWS_RESOURCES}
)

# Add icon
#
if (MSVC)
	set (WINDOWS_RESOURCES ./WinResources/WinResources.rc)
endif (MSVC)

target_precompile_headers(${TARGET} PRIVATE
	# C includes, must be before c++ includes
	#
	<cassert>
	<cstdint>

	# C++ includes
	#
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

	<QApplication>
	<QHBoxLayout>
	<QLabel>
	<QMenuBar>
	<QPushButton>
	<QStatusBar>
	<QVBoxLayout>
	<QWidget>

	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	
	<ProtoSerialization.h>
	<Network.pb.h>
	
	"../../AppSignalLib/AppSignal.h"
)

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Network Qt6::Widgets Qt6::Qml Qt6::Xml)
target_link_libraries(${TARGET} PRIVATE CommonLib ProtobufEx UtilsLib AppSignalLib OnlineLib HardwareLib ServiceLib)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()

add_code_coverage_options(${TARGET} CODE_COVERAGE)
