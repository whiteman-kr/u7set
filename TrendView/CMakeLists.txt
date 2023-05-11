cmake_minimum_required(VERSION 3.16)

project(TrendView LANGUAGES CXX)
message("${PROJECT_NAME}, Version: ${CMAKE_PROJECT_VERSION}")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Svg Widgets PrintSupport)

add_library(TrendView STATIC
	../lib/ISignalDataServer.h
	../lib/ISignalHasTag.h
	DialogChooseTrendSignals.cpp
	DialogChooseTrendSignals.h
	DialogTrendSignalPoint.cpp
	DialogTrendSignalPoint.h
	DialogTrendSignalPoints.cpp
	DialogTrendSignalPoints.h
	DialogTrendSignalProperties.cpp
	DialogTrendSignalProperties.h
	ITrendDataProvider.h
	Trend.cpp
	Trend.h
	TrendArchiveServer.cpp
	TrendArchiveServer.h
	TrendMainWindow.cpp
	TrendMainWindow.h
	TrendParam.cpp
	TrendParam.h
	TrendRuler.cpp
	TrendRuler.h
	TrendScale.cpp
	TrendScale.h
	TrendSettings.cpp
	TrendSettings.h
	TrendSignal.cpp
	TrendSignal.h
	TrendSignalSet.cpp
	TrendSignalSet.h
	TrendSignalState.cpp
	TrendSignalState.h
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
)

target_link_libraries(TrendView PUBLIC Qt6::Gui Qt6::Svg Qt6::Widgets Qt6::PrintSupport)
target_link_libraries(TrendView PUBLIC CommonLib Protobuf UtilsLib AppSignalLib )

target_compile_definitions(TrendView PRIVATE TREND_VIEW_LIBRARY)
#target_include_directories(TrendView INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

