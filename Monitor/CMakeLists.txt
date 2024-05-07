cmake_minimum_required(VERSION 3.16)

project(Monitor LANGUAGES CXX)

include(../cmake/vld.cmake)
include(../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../lib/Ui)

if (MSVC)
  # MSVC warning suppressions
  add_compile_options(
	/wd4828 # The file contains a character starting at offset 0x10d2 that is illegal in the current source character set (codepage 65001).
  )
endif (MSVC)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Qml Xml Svg PrintSupport LinguistTools)
qt_standard_project_setup()

set (MONITOR_SOURCES
	../lib/ConstStrings.h
	../lib/ISignalDataServer.h
	../lib/Ui/DialogAbout.cpp
	../lib/Ui/DialogAbout.h
	../lib/Ui/DialogAlert.cpp
	../lib/Ui/DialogAlert.h
	../lib/Ui/DialogSignalInfo.cpp
	../lib/Ui/DialogSignalInfo.h
	../lib/Ui/DialogSignalSearch.cpp
	../lib/Ui/DialogSignalSearch.h
	../lib/Ui/DialogWriteValues.cpp
	../lib/Ui/DialogWriteValues.h
	Archive/ArchiveConnection.cpp
	Archive/ArchiveConnection.h
	Archive/ArchiveData.cpp
	Archive/ArchiveData.h
	Archive/ArchiveModelView.cpp
	Archive/ArchiveModelView.h
	Archive/ArchiveTcpClient2.cpp
	Archive/ArchiveTcpClient2.h
	Archive/ArchiveWidget.cpp
	Archive/ArchiveWidget.h
	Archive/DialogChooseArchiveSignals.cpp
	Archive/DialogChooseArchiveSignals.h
	Archive/MonitorArchive.cpp
	Archive/MonitorArchive.h
	DataSourcesWidget.cpp
	DataSourcesWidget.h
	DevTools/DevToolsGlobalScript.cpp
	DevTools/DevToolsGlobalScript.h
	DevTools/DevToolsSchemaStats.cpp
	DevTools/DevToolsSchemaStats.h
	DevTools/DevToolsScriptVariables.cpp
	DevTools/DevToolsScriptVariables.h
	DevTools/DevToolsSettings.cpp
	DevTools/DevToolsSettings.h	
	DevTools/DevToolsViewVariables.cpp
	DevTools/DevToolsViewVariables.h
	DialogColumns.cpp
	DialogColumns.h
	DialogSettings.cpp
	DialogSettings.h
	Globals.cpp
	Globals.h
	main.cpp
	MonitorAppSettings.cpp
	MonitorAppSettings.h
	MonitorCentralWidget.h
	MonitorConfigController.cpp
	MonitorConfigController.h
	MonitorMainWindow.cpp
	MonitorMainWindow.h
	MonitorSchemaManager.cpp
	MonitorSchemaManager.h
	MonitorSchemaView.cpp
	MonitorSchemaView.h
	MonitorSchemaWidget.cpp
	MonitorSchemaWidget.h
	MonitorSignalInfo.cpp
	MonitorSignalInfo.h
	MonitorSignalSnapshot.cpp
	MonitorSignalSnapshot.h
	ScriptMonitorApplication.cpp
	ScriptMonitorApplication.h
	SelectSchemaWidget.cpp
	SelectSchemaWidget.h
	Trend/ArchiveTrendTcpClient.cpp
	Trend/ArchiveTrendTcpClient.h
	Trend/MonitorTrendArchiveConnections.cpp
	Trend/MonitorTrendArchiveConnections.h
	Trend/MonitorTrends.cpp
	Trend/MonitorTrends.h
	Trend/RtSchemaTrend.cpp
	Trend/RtSchemaTrend.h
)

set (MONITOR_RESOURCES
	Monitor.qrc
	../Etc/Logo/RadiyLogo.qrc
)

# Add icon
#
if (MSVC)
	list (APPEND MONITOR_RESOURCES ./WinResources/WinResources.rc)
endif (MSVC)

set (MONITOR_FORMS
	${CMAKE_CURRENT_SOURCE_DIR}/../lib/Ui/DialogSignalInfo.ui
	./Archive/DialogChooseArchiveSignals.ui
	DialogSettings.ui
	DialogColumns.ui
)

set(TARGET Monitor)

add_executable(${TARGET} WIN32
	${MONITOR_FORMS}
	${MONITOR_SOURCES}
	${MONITOR_RESOURCES}
)

#Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_precompile_headers(${TARGET} PRIVATE
	# C includes, must be before c++ includes
	#
	<cassert>
	<cstdint>

	# C++ includes
	#
	<algorithm>
	<array>
	<atomic>
	<concepts>
	<deque>
	<functional>
	<limits>
	<list>
	<map>
	<memory>
	<queue>
	<ranges>
	<set>
	<type_traits>
	<vector>

	# Qt includes
	#
	<QTreeWidget>
	<QTreeView>
	<QtPrintSupport>
	<QToolBar>
	<QStatusBar>
	<QPushButton>
	<QLineEdit>
	<QLabel>
	<QFileDialog>
	<QDialog>
	<QDateTime>
	<QJSEngine>

	# Project includes
	#
	"../CommonLib/Types.h"
	"../CommonLib/Hash.h"
	"../CommonLib/PropertyObject.h"

	<ProtoSerialization.h>
	<Network.pb.h>
	<AppSignal.pb.h>

	"../AppSignalLib/AppSignal.h"
	"../AppSignalLib/AppSignalParam.h"
	"../AppSignalLib/AppSignalState.h"
)

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Qml Qt6::Xml Qt6::Svg Qt6::PrintSupport)
target_link_libraries(${TARGET} PRIVATE CommonLib Behavior Protobuf UtilsLib AppSignalLib OnlineLib TrendView VFrame30 ClientLib 
	SchemaClientLib	UiLib)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	 target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()

