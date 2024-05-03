cmake_minimum_required(VERSION 3.16)

project(VFrame30 LANGUAGES CXX)

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

qt_add_library(${TARGET} STATIC
	../lib/Tuning/ITuningConnection.h

	./SchemaItems/FblItem.cpp
	./SchemaItems/FblItem.h
	./SchemaItems/FblItemLine.cpp
	./SchemaItems/FblItemLine.h
	./SchemaItems/FblItemRect.cpp
	./SchemaItems/FblItemRect.h
	./SchemaItems/ImageItem.cpp
	./SchemaItems/ImageItem.h
	./SchemaItems/Indicator.cpp
	./SchemaItems/Indicator.h
	./SchemaItems/IndicatorArrowIndicator.cpp
	./SchemaItems/IndicatorArrowIndicator.h
	./SchemaItems/IndicatorHistogramVert.cpp
	./SchemaItems/IndicatorHistogramVert.h
	./SchemaItems/IndicatorTrend.cpp
	./SchemaItems/IndicatorTrend.h
	./SchemaItems/PosConnectionImpl.cpp
	./SchemaItems/PosConnectionImpl.h
	./SchemaItems/PosLineImpl.cpp
	./SchemaItems/PosLineImpl.h
	./SchemaItems/PosRectImpl.cpp
	./SchemaItems/PosRectImpl.h
	./SchemaItems/PosRectRotatable.cpp
	./SchemaItems/PosRectRotatable.h
	./SchemaItems/SchemaItem.cpp
	./SchemaItems/SchemaItem.h
	./SchemaItems/SchemaItemAfb.cpp
	./SchemaItems/SchemaItemAfb.h
	./SchemaItems/SchemaItemBus.cpp
	./SchemaItems/SchemaItemBus.h
	./SchemaItems/SchemaItemConnection.cpp
	./SchemaItems/SchemaItemConnection.h
	./SchemaItems/SchemaItemConst.cpp
	./SchemaItems/SchemaItemConst.h
	./SchemaItems/SchemaItemControl.cpp
	./SchemaItems/SchemaItemControl.h
	./SchemaItems/SchemaItemDiagValue.cpp
	./SchemaItems/SchemaItemDiagValue.h
	./SchemaItems/SchemaItemFrame.cpp
	./SchemaItems/SchemaItemFrame.h
	./SchemaItems/SchemaItemImage.cpp
	./SchemaItems/SchemaItemImage.h
	./SchemaItems/SchemaItemImageValue.cpp
	./SchemaItems/SchemaItemImageValue.h
	./SchemaItems/SchemaItemIndicator.cpp
	./SchemaItems/SchemaItemIndicator.h
	./SchemaItems/SchemaItemLine.cpp
	./SchemaItems/SchemaItemLine.h
	./SchemaItems/SchemaItemLineEdit.cpp
	./SchemaItems/SchemaItemLineEdit.h
	./SchemaItems/SchemaItemLink.cpp
	./SchemaItems/SchemaItemLink.h
	./SchemaItems/SchemaItemLoopback.cpp
	./SchemaItems/SchemaItemLoopback.h
	./SchemaItems/SchemaItemPath.cpp
	./SchemaItems/SchemaItemPath.h
	./SchemaItems/SchemaItemPushButton.cpp
	./SchemaItems/SchemaItemPushButton.h
	./SchemaItems/SchemaItemRect.cpp
	./SchemaItems/SchemaItemRect.h
	./SchemaItems/SchemaItemSignal.cpp
	./SchemaItems/SchemaItemSignal.h
	./SchemaItems/SchemaItemSlider.cpp
	./SchemaItems/SchemaItemSlider.h
	./SchemaItems/SchemaItemTerminator.cpp
	./SchemaItems/SchemaItemTerminator.h
	./SchemaItems/SchemaItemUfb.cpp
	./SchemaItems/SchemaItemUfb.h
	./SchemaItems/SchemaItemValue.cpp
	./SchemaItems/SchemaItemValue.h
	./SchemaItems/SchemaItemVduLine.cpp
	./SchemaItems/SchemaItemVduLine.h
	./SchemaItems/SchemaItemVduRect.cpp
	./SchemaItems/SchemaItemVduRect.h
	./SchemaItems/SchemaItemVduValue.cpp
	./SchemaItems/SchemaItemVduValue.h

	AppSignalController.cpp
	AppSignalController.h
	BaseSchemaWidget.cpp
	BaseSchemaWidget.h
	ClientSchemaView.cpp
	ClientSchemaView.h
	ClientSchemaWidget.cpp
	ClientSchemaWidget.h
	Context.cpp
	Context.h
	DiagSchema.cpp
	DiagSchema.h
	DiagStateController.cpp
	DiagStateController.h
	DrawParam.cpp
	DrawParam.h
	FontParam.cpp
	FontParam.h
	HorzVertLinks.cpp
	HorzVertLinks.h
	IMatsSchemaItemAssociations.h
	ISchemaViewHistory.h
	ITimeStats.h
	IViewVariables.h
	LogController.cpp
	LogController.h
	LogicSchema.cpp
	LogicSchema.h
	MacrosExpander.cpp
	MacrosExpander.h
	MonitorSchema.cpp
	MonitorSchema.h
	PropertyNames.cpp
	PropertyNames.h
	Schema.cpp
	Schema.h
	SchemaDetails.cpp
	SchemaDetails.h
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
	VduSchema.cpp
	VduSchema.h
	VFrame30Library.cpp
	VFrame30Library.h
	VFrame30Types.h
	VFrameTools.cpp
	VFrameTools.h
	WiringSchema.cpp
	WiringSchema.h
)

target_precompile_headers(${TARGET} PRIVATE
		<algorithm>
		<array>
		<cassert>
		<cmath>
		<chrono>
		<fstream>
		<functional>
		<iostream>
		<iterator>
		<list>
		<map>
		<memory>
		<optional>
		<optional>
		<ranges>
		<set>
		<string>
		<type_traits>
		<unordered_map>
		<unordered_set>
		<utility>
		<variant>
		<vector>

		<QAction>
		<QApplication>
		<QDebug>
		<QDomElement>
		<QDrag>
		<qglobal.h>
		<QJSEngine>
		<QJsonDocument>
		<QJsonObject>
		<QLineEdit>
		<QMainWindow>
		<QMessageBox>
		<QMouseEvent>
		<QMutex>
		<QPainter>
		<QPainterPath>
		<QPdfWriter>
		<QPushButton>
		<QJSEngine>
		<QRect>
		<QScreen>
		<QScrollArea>
		<QScrollBar>
		<QSlider>
		<QStaticText>
		<QStyleOptionButton>
		<QSvgRenderer>
		<QTextDocument>
		<QUuid>
		<QWheelEvent>
		<QWindow>
		<QXmlStreamReader>

		<QtConcurrent/QtConcurrent>

		"../CommonLib/Types.h"
		"../CommonLib/Hash.h"
		"../CommonLib/PropertyObject.h"
		"../CommonLib/AfbParamValue.h"

		<ProtoSerialization.h>
		<ProtoCommonHelper.h>
		<VFrame30.pb.h>

		"../AppSignalLib/AppSignalParam.h"
		"../AppSignalLib/AppSignalState.h"
		"../AppSignalLib/TuningValue.h"

		"VFrameTools.h"
)

target_link_libraries(${TARGET} PUBLIC Qt6::Widgets Qt6::Qml Qt6::Xml Qt6::Svg)
target_link_libraries(${TARGET} PUBLIC CommonLib Behavior Protobuf UtilsLib AppSignalLib HardwareLib DiagStateLib TrendView)

target_compile_definitions(${TARGET} PRIVATE VFRAME30_LIBRARY)
#target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)
