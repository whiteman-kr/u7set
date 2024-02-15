cmake_minimum_required(VERSION 3.16)

project(SchemaClientLib LANGUAGES CXX)

include(../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Xml Network Widgets LinguistTools Qml)

set(TARGET SchemaClientLib)

qt_add_library(${TARGET} STATIC
	ClientSchemaManager.cpp
	ClientSchemaManager.h
	SchemaClientConfigController.cpp
	SchemaClientConfigController.h
	SchemaTabWidget.cpp
	SchemaTabWidget.h
	SchemaDrawStatistics.cpp
	SchemaDrawStatistics.h

	../lib/Ui/TabWidgetEx.cpp  # Note that is usend in other projects without this library
	../lib/Ui/TabWidgetEx.h
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
	<QMessageBox>
	<QPushButton>
	<QReadWriteLock>
	<QSharedMemory>
	<QTimeZone>
	<QTranslator>
	<QVBoxLayout>

	# Project headers
	#
	"../CommonLib/Types.h"
	"../CommonLib/Hash.h"
	"../CommonLib/PropertyObject.h"

	"../Proto/ProtoSerialization.h"

	"../AppSignalLib/AppSignalParam.h"
	"../AppSignalLib/AppSignalState.h"
)

#Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PUBLIC Qt6::Gui Qt6::Xml Qt6::Network Qt6::Widgets Qt6::Qml)
target_link_libraries(${TARGET} PUBLIC CommonLib UtilsLib ClientLib VFrame30)

target_compile_definitions(${TARGET} PRIVATE SCHEMACLIENT_LIB_DOMAIN)
#target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)
