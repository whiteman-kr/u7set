cmake_minimum_required(VERSION 3.16)

project(PacketViewer LANGUAGES CXX)

include(../../cmake/vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Qml Xml Concurrent)
qt_standard_project_setup()

set(TARGET PacketViewer)

add_executable(${TARGET} WIN32
	main.cpp
	PacketBufferTableModel.cpp
	PacketBufferTableModel.h
	PacketSourceModel.cpp
	PacketSourceModel.h
	SendTuningFrameWidget.cpp
	SendTuningFrameWidget.h
	SignalTableModel.cpp
	SignalTableModel.h
	SourceListWidget.cpp
	SourceListWidget.h
	SourceStatusWidget.cpp
	SourceStatusWidget.h
)

target_precompile_headers(${TARGET} PRIVATE
	# C/C++ includes
	#
	<cassert>
	<cstdint>
	<memory>
	<vector>
	<list>
	<map>
	<unordered_map>
	<set>
	<algorithm>
	<functional>
	<type_traits>
	<limits>

	# Qt includes
	#
	#<QtGui>
	#<QtWidgets>
	#<QtNetwork>
	#<QtConcurrent>
	#<QtQml>

	# Project includes
	#
	"../../CommonLib/Types.h"
	"../../CommonLib/Hash.h"
	#"../../CommonLib/PropertyObject.h"

	<ProtoSerialization.h>
	<Network.pb.h>
	#<AppSignal.pb.h>

	"../../AppSignalLib/AppSignal.h"
	#"../../AppSignalLib/AppSignalParam.h"
	#"../../AppSignalLib/AppSignalState.h"
)

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Qml Qt6::Xml Qt6::Concurrent)
target_link_libraries(${TARGET} PRIVATE CommonLib Protobuf UtilsLib AppSignalLib OnlineLib HardwareLib)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	 target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()

