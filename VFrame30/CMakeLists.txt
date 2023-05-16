cmake_minimum_required(VERSION 3.16)

project(VFrame30 LANGUAGES CXX)
message("${PROJECT_NAME}, Version: ${CMAKE_PROJECT_VERSION}")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Qml Xml Svg)

if (MSVC)
  # MSVC warning suppressions
  add_compile_options(
	/wd4828 # The file contains a character starting at offset 0x10d2 that is illegal in the current source character set (codepage 65001).
  )
endif (MSVC)

set(TARGET VFrame30)

add_library(${TARGET} STATIC
	../lib/ClientBehavior.cpp
	../lib/ClientBehavior.h
	../lib/ITimeStats.h
	../lib/Tuning/ITuningConnection.h
	AppSignalController.cpp
	AppSignalController.h
	BaseSchemaWidget.cpp
	BaseSchemaWidget.h
	Bus.cpp
	Bus.h
	ClientSchemaView.cpp
	ClientSchemaView.h
	ClientSchemaWidget.cpp
	ClientSchemaWidget.h
	Configuration.cpp
	Configuration.h
	Context.cpp
	Context.h
	DiagSchema.cpp
	DiagSchema.h
	DrawParam.cpp
	DrawParam.h
	FblItem.cpp
	FblItem.h
	FblItemLine.cpp
	FblItemLine.h
	FblItemRect.cpp
	FblItemRect.h
	FontParam.cpp
	FontParam.h
	HorzVertLinks.cpp
	HorzVertLinks.h
	ImageItem.cpp
	ImageItem.h
	Indicator.cpp
	Indicator.h
	IndicatorArrowIndicator.cpp
	IndicatorArrowIndicator.h
	IndicatorHistogramVert.cpp
	IndicatorHistogramVert.h
	IndicatorTrend.cpp
	IndicatorTrend.h
	ISchemaViewHistory.h
	IViewVariables.h
	LogController.cpp
	LogController.h
	LogicSchema.cpp
	LogicSchema.h
	MacrosExpander.cpp
	MacrosExpander.h
	MonitorSchema.cpp
	MonitorSchema.h
	PosConnectionImpl.cpp
	PosConnectionImpl.h
	PosLineImpl.cpp
	PosLineImpl.h
	PosRectImpl.cpp
	PosRectImpl.h
	PropertyNames.cpp
	PropertyNames.h
	Schema.cpp
	Schema.h
	SchemaItem.cpp
	SchemaItem.h
	SchemaItemAfb.cpp
	SchemaItemAfb.h
	SchemaItemBus.cpp
	SchemaItemBus.h
	SchemaItemConnection.cpp
	SchemaItemConnection.h
	SchemaItemConst.cpp
	SchemaItemConst.h
	SchemaItemControl.cpp
	SchemaItemControl.h
	SchemaItemFrame.cpp
	SchemaItemFrame.h
	SchemaItemImage.cpp
	SchemaItemImage.h
	SchemaItemImageValue.cpp
	SchemaItemImageValue.h
	SchemaItemIndicator.cpp
	SchemaItemIndicator.h
	SchemaItemLine.cpp
	SchemaItemLine.h
	SchemaItemLineEdit.cpp
	SchemaItemLineEdit.h
	SchemaItemLink.cpp
	SchemaItemLink.h
	SchemaItemLoopback.cpp
	SchemaItemLoopback.h
	SchemaItemPath.cpp
	SchemaItemPath.h
	SchemaItemPushButton.cpp
	SchemaItemPushButton.h
	SchemaItemRect.cpp
	SchemaItemRect.h
	SchemaItemSignal.cpp
	SchemaItemSignal.h
	SchemaItemTerminator.cpp
	SchemaItemTerminator.h
	SchemaItemUfb.cpp
	SchemaItemUfb.h
	SchemaItemValue.cpp
	SchemaItemValue.h
	SchemaLayer.cpp
	SchemaLayer.h
	SchemaManager.cpp
	SchemaManager.h
	SchemaPoint.cpp
	SchemaPoint.h
	SchemaView.cpp
	SchemaView.h
	Session.cpp
	Session.h
	Settings.cpp
	Settings.h
	TuningController.cpp
	TuningController.h
	TuningSchema.cpp
	TuningSchema.h
	UfbSchema.cpp
	UfbSchema.h
	VFrame30Library.cpp
	VFrame30Library.h
	VFrameTools.cpp
	VFrameTools.h
	WiringSchema.cpp
	WiringSchema.h
)

target_precompile_headers(${TARGET} PUBLIC
	    <array>
		<iostream>
		<fstream>
		<list>
		<vector>
		<map>
		<unordered_map>
		<unordered_set>
		<set>
		<cmath>
		<cassert>
		<memory>
		<utility>
		<string>
		<optional>
		<variant>
		<algorithm>
		<functional>
		<optional>
		<iterator>
		<type_traits>
		<ranges>

		<qglobal.h>
		<QApplication>
		<QMainWindow>
		<QWindow>
		<QScrollArea>
		<QScrollBar>
		<QMessageBox>
		<QPushButton>
		<QLineEdit>
		<QtConcurrent/QtConcurrent>
		<QPainter>
		<QMouseEvent>
		<QWheelEvent>
		<QStaticText>
		<QScreen>
		<QUuid>
		<QDebug>
		<QRect>
		<QMutex>
		<QJSEngine>
		<QQmlEngine>
		<QAction>
		<QDrag>
		<QPdfWriter>
		<QXmlStreamReader>
		<QDomElement>
		<QJsonDocument>
		<QJsonObject>

		"VFrameTools.h"
		"../Proto/ProtoSerialization.h"
)

target_link_libraries(${TARGET} PUBLIC Qt6::Widgets Qt6::Qml Qt6::Xml Qt6::Svg)
target_link_libraries(${TARGET} PUBLIC CommonLib Protobuf UtilsLib AppSignalLib HardwareLib TrendView)

target_compile_definitions(${TARGET} PRIVATE VFRAME30_LIBRARY)
#target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})


