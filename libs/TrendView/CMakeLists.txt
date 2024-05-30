cmake_minimum_required(VERSION 3.16)

project(TrendView LANGUAGES CXX)

include(../../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Svg Widgets PrintSupport LinguistTools)

set(TARGET TrendView)

qt_add_library(${TARGET} STATIC
	../../lib/ISignalDataServer.h
	../../lib/ISignalHasTag.h
	./include/TrendView/DialogChooseTrendSignals.h
	./include/TrendView/ITrendDataProvider.h
	./include/TrendView/Trend.h
	./include/TrendView/TrendArchiveServer.h
	./include/TrendView/TrendMainWindow.h
	./include/TrendView/TrendParam.h
	./include/TrendView/TrendSignal.h
	./include/TrendView/TrendSignalSet.h
	./include/TrendView/TrendSignalState.h
	DialogChooseTrendSignals.cpp
	DialogTrendSignalPoint.cpp
	DialogTrendSignalPoint.h
	DialogTrendSignalPoints.cpp
	DialogTrendSignalPoints.h
	DialogTrendSignalProperties.cpp
	DialogTrendSignalProperties.h
	Trend.cpp
	TrendArchiveServer.cpp
	TrendImpl.cpp
	TrendImpl.h
	TrendMainWindow.cpp
	TrendParam.cpp
	TrendRuler.cpp
	TrendRuler.h
	TrendScale.cpp
	TrendScale.h
	TrendSettings.cpp
	TrendSettings.h
	TrendSignal.cpp
	TrendSignalSet.cpp
	TrendSignalState.cpp
	TrendSlider.cpp
	TrendSlider.h
	TrendWidget.cpp
	TrendWidget.h

	DialogChooseTrendSignals.ui
	TrendsMainWindow.ui
	DialogTrendSignalProperties.ui
	DialogTrendSignalPoints.ui
	DialogTrendSignalPoint.ui

	TrendView.qrc
	../../Etc/Logo/RadiyLogo.qrc
)

target_precompile_headers(${TARGET} PRIVATE
	<QAction>
	<QActionGroup>
	<QComboBox>
	<QDialogButtonBox>
	<QElapsedTimer>
	<QGridLayout>
	<QMessageBox>
	<QPainter>
	<QPrintDialog>
	<QPrinter>
	<QPushButton>
	<QScreen>
	<QToolBar>
	<QFile>

	"../../CommonLib/Types.h"
	"../../CommonLib/Hash.h"

	<Trends.pb.h>

	"../../AppSignalLib/AppSignalParam.h"
	"../../AppSignalLib/AppSignalState.h"
)

#Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PUBLIC Qt6::Gui Qt6::Svg Qt6::Widgets Qt6::PrintSupport)
target_link_libraries(${TARGET} PUBLIC CommonLib Protobuf UtilsLib AppSignalLib AppSignalLists)

target_compile_definitions(${TARGET} PRIVATE TREND_VIEW_LIBRARY)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
