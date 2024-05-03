cmake_minimum_required(VERSION 3.16)

project(SchemaClientLib LANGUAGES CXX)

include(../../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Xml Network Widgets LinguistTools Qml)

set(TARGET SchemaClientLib)

qt_add_library(${TARGET} STATIC
	./include/SchemaClientLib/AppDataSourcesWidget.h
	./include/SchemaClientLib/ClientSchemaManager.h
	./include/SchemaClientLib/DevToolsWindow.h
	./include/SchemaClientLib/DialogTcpStatistics.h
	./include/SchemaClientLib/IDevTools.h
	./include/SchemaClientLib/SchemaClientConfigController.h
	./include/SchemaClientLib/SchemaDrawStatistics.h
	./include/SchemaClientLib/SchemaTabWidget.h
	./include/SchemaClientLib/TuningSourcesWidget.h
	AppDataSourcesWidget.cpp
	ClientSchemaManager.cpp
	DevToolsAppSettings.cpp
	DevToolsAppSettings.h
	DevToolsConnections.cpp
	DevToolsConnections.h
	DevToolsGlobalScript.cpp
	DevToolsGlobalScript.h
	DevToolsSchemaStats.cpp
	DevToolsSchemaStats.h
	DevToolsScriptVariables.cpp
	DevToolsScriptVariables.h
	DevToolsViewVariables.cpp
	DevToolsViewVariables.h
	DevToolsWidget.cpp
	DevToolsWidget.h
	DevToolsWindow.cpp
	DialogSourceInfo.cpp
	DialogSourceInfo.h
	DialogTcpStatistics.cpp
	SchemaClientConfigController.cpp
	SchemaDrawStatistics.cpp
	SchemaTabWidget.cpp
	TcpStatisticsWidget.cpp
	TcpStatisticsWidget.h
	TuningSourcesWidget.cpp

	../../lib/Ui/TabWidgetEx.cpp  # !!! Note that is used in other projects without this library !!!
	../../lib/Ui/TabWidgetEx.h
)

target_precompile_headers(${TARGET} PRIVATE
	<algorithm>
	<cassert>
	<chrono>
	<concepts>
	<cstdint>
	<functional>
	<list>
	<map>
	<memory>
	<mutex>
	<queue>
	<ranges>
	<set>
	<unordered_map>
	<unordered_set>
	<vector>

	<QAction>
	<QApplication>
	<QComboBox>
	<QCoreApplication>
	<QDateTime>
	<QDialog>
	<QDomDocument>
	<QHBoxLayout>
	<QJSEngine>
	<QLabel>
	<QLineEdit>
	<QLocale>
	<QMenu>
	<QMessageBox>
	<QPushButton>
	<QReadWriteLock>
	<QSharedMemory>
	<QTabWidget>
	<QTextEdit>
	<QTimer>
	<QTimeZone>
	<QTranslator>
	<QTreeWidget>
	<QVBoxLayout>

	# Project headers
	#
	"../../CommonLib/Types.h"
	"../../CommonLib/Hash.h"
	"../../CommonLib/PropertyObject.h"

	<ProtoSerialization.h>
	<Network.pb.h>

	"../../AppSignalLib/AppSignalParam.h"
	"../../AppSignalLib/AppSignalState.h"
)

target_compile_definitions(${TARGET} PRIVATE SCHEMACLIENT_LIB_DOMAIN)

#Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PUBLIC Qt6::Gui Qt6::Xml Qt6::Network Qt6::Widgets Qt6::Qml)
target_link_libraries(${TARGET} PUBLIC CommonLib UtilsLib OnlineLib ClientLib VFrame30)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
