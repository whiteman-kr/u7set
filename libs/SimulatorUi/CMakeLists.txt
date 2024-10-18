cmake_minimum_required(VERSION 3.16)

project(VFrame30 LANGUAGES CXX)

include(../../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../SchemaClientLib)

find_package(Qt6 REQUIRED COMPONENTS Widgets Qml Xml Svg)

set(TARGET SimulatorUi)

qt_add_library(${TARGET} STATIC
	./include/SimulatorUi/ISimPropertyStorage.h
	./include/SimulatorUi/SimWidget.h
	./include/SimulatorUi/SimIdeSimulator.h

	./src/SimWidgetPrivate.h
	./src/SimWidgetPrivate.cpp
	./src/SimWidget.cpp
	./src/SimTrends.h
	./src/SimTrends.cpp
	./src/SimSignalSnapshot.h
	./src/SimSignalSnapshot.cpp
	./src/SimSignalInfo.h
	./src/SimSignalInfo.cpp
	./src/SimSelectSchemaPage.h
	./src/SimSelectSchemaPage.cpp
	./src/SimSchemaWidget.h
	./src/SimSchemaWidget.cpp
	./src/SimSchemaView.h
	./src/SimSchemaView.cpp
	./src/SimSchemaPage.h
	./src/SimSchemaPage.cpp
	./src/SimSchemaManager.h
	./src/SimSchemaManager.cpp
	./src/SimProjectWidget.h
	./src/SimProjectWidget.cpp
	./src/SimOverrideValueWidget.h
	./src/SimOverrideValueWidget.cpp
	./src/SimOverridePane.h
	./src/SimOverridePane.cpp
	./src/SimOutputWidget.h
	./src/SimOutputWidget.cpp
	./src/SimLogicModulePage.h
	./src/SimLogicModulePage.cpp
	./src/SimIdeSimulator.cpp
	./src/SimConnectionPage.h
	./src/SimConnectionPage.cpp
	./src/SimCodePage.h
	./src/SimCodePage.cpp
	./src/SimBasePage.h
	./src/SimBasePage.cpp
	./src/ScriptSimApplication.h
	./src/ScriptSimApplication.cpp

	./src/SimulatorUi.qrc
)

target_precompile_headers(${TARGET} PRIVATE
		<QAction>
		<QDialog>
		<QLabel>
		<QMessageBox>
		<QMenu>
		<QPushButton>
		<QSettings>

		<CommonLib/Types.h>
		<CommonLib/Hash.h>
		<CommonLib/PropertyObject.h>

		<ProtoSerialization.h>
		<ProtoCommonHelper.h>
		<AppSignal.pb.h>

		"../../AppSignalLib/AppSignalParam.h"
		"../../AppSignalLib/AppSignalState.h"
		"../../AppSignalLib/TuningValue.h"
)

add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PUBLIC Qt6::Widgets Qt6::Qml Qt6::Xml Qt6::Svg)
target_link_libraries(${TARGET} PUBLIC SimulatorLib AppSignalLists CommonLib HardwareLib VFrame30 SchemaClientLib UiLib UtilsLib TrendView)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

