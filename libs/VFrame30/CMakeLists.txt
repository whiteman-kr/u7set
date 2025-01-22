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
	./include/VFrame30/AppSignalController.h
	./include/VFrame30/BaseSchemaWidget.h
	./include/VFrame30/ClientSchemaView.h
	./include/VFrame30/ClientSchemaWidget.h
	./include/VFrame30/Context.h
	./include/VFrame30/DiagSchema.h
	./include/VFrame30/DiagStateController.h
	./include/VFrame30/DrawParam.h
	./include/VFrame30/FblItem.h
	./include/VFrame30/FblItemLine.h
	./include/VFrame30/FblItemRect.h
	./include/VFrame30/FontParam.h
	./include/VFrame30/HorzVertLinks.h
	./include/VFrame30/ImageItem.h
	./include/VFrame30/IMatsSchemaItemAssociations.h
	./include/VFrame30/Indicator.h
	./include/VFrame30/IndicatorArrowIndicator.h
	./include/VFrame30/IndicatorHistogramVert.h
	./include/VFrame30/IndicatorTrend.h
	./include/VFrame30/ISchemaViewHistory.h
	./include/VFrame30/ITimeStats.h
	./include/VFrame30/IViewVariables.h
	./include/VFrame30/LogController.h
	./include/VFrame30/LogicSchema.h
	./include/VFrame30/MacrosExpander.h
	./include/VFrame30/MonitorSchema.h
	./include/VFrame30/PosConnectionImpl.h
	./include/VFrame30/PosLineImpl.h
	./include/VFrame30/PosRectImpl.h
	./include/VFrame30/PosRectRotatable.h
	./include/VFrame30/PropertyNames.h
	./include/VFrame30/Schema.h
	./include/VFrame30/SchemaDetails.h
	./include/VFrame30/SchemaItem.h
	./include/VFrame30/SchemaItemAfb.h
	./include/VFrame30/SchemaItemBus.h
	./include/VFrame30/SchemaItemConnection.h
	./include/VFrame30/SchemaItemConst.h
	./include/VFrame30/SchemaItemControl.h
	./include/VFrame30/SchemaItemDiagValue.h
	./include/VFrame30/SchemaItemFrame.h
	./include/VFrame30/SchemaItemImage.h
	./include/VFrame30/SchemaItemImageValue.h
	./include/VFrame30/SchemaItemIndicator.h
	./include/VFrame30/SchemaItemLine.h
	./include/VFrame30/SchemaItemLineEdit.h
	./include/VFrame30/SchemaItemLink.h
	./include/VFrame30/SchemaItemLoopback.h
	./include/VFrame30/SchemaItemPath.h
	./include/VFrame30/SchemaItemPushButton.h
	./include/VFrame30/SchemaItemRect.h
	./include/VFrame30/SchemaItemSignal.h
	./include/VFrame30/SchemaItemSlider.h
	./include/VFrame30/SchemaItemTerminator.h
	./include/VFrame30/SchemaItemUfb.h
	./include/VFrame30/SchemaItemValue.h
	./include/VFrame30/SchemaItemVdu.h
	./include/VFrame30/SchemaItemVduImage.h
	./include/VFrame30/SchemaItemVduLine.h
	./include/VFrame30/SchemaItemVduRect.h
	./include/VFrame30/SchemaItemVduValue.h
	./include/VFrame30/SchemaLayer.h
	./include/VFrame30/SchemaManager.h
	./include/VFrame30/SchemaPoint.h
	./include/VFrame30/SchemaView.h
	./include/VFrame30/Session.h
	./include/VFrame30/Settings.h
	./include/VFrame30/TuningController.h
	./include/VFrame30/TuningSchema.h
	./include/VFrame30/UfbSchema.h
	./include/VFrame30/VduItemVisitor.h
	./include/VFrame30/VduSchema.h
	./include/VFrame30/VFrame30Library.h
	./include/VFrame30/VFrame30Types.h
	./include/VFrame30/VFrameTools.h
	./include/VFrame30/WiringSchema.h

	./SchemaItems/FblItem.cpp
	./SchemaItems/FblItemLine.cpp
	./SchemaItems/FblItemRect.cpp
	./SchemaItems/ImageItem.cpp
	./SchemaItems/Indicator.cpp
	./SchemaItems/IndicatorArrowIndicator.cpp
	./SchemaItems/IndicatorHistogramVert.cpp
	./SchemaItems/IndicatorTrend.cpp
	./SchemaItems/PosConnectionImpl.cpp
	./SchemaItems/PosLineImpl.cpp
	./SchemaItems/PosRectImpl.cpp
	./SchemaItems/PosRectRotatable.cpp
	./SchemaItems/SchemaItem.cpp
	./SchemaItems/SchemaItemAfb.cpp
	./SchemaItems/SchemaItemBus.cpp
	./SchemaItems/SchemaItemConnection.cpp
	./SchemaItems/SchemaItemConst.cpp
	./SchemaItems/SchemaItemControl.cpp
	./SchemaItems/SchemaItemDiagValue.cpp
	./SchemaItems/SchemaItemFrame.cpp
	./SchemaItems/SchemaItemImage.cpp
	./SchemaItems/SchemaItemImageValue.cpp
	./SchemaItems/SchemaItemIndicator.cpp
	./SchemaItems/SchemaItemLine.cpp
	./SchemaItems/SchemaItemLineEdit.cpp
	./SchemaItems/SchemaItemLink.cpp
	./SchemaItems/SchemaItemLoopback.cpp
	./SchemaItems/SchemaItemPath.cpp
	./SchemaItems/SchemaItemPushButton.cpp
	./SchemaItems/SchemaItemRect.cpp
	./SchemaItems/SchemaItemSignal.cpp
	./SchemaItems/SchemaItemSlider.cpp
	./SchemaItems/SchemaItemTerminator.cpp
	./SchemaItems/SchemaItemUfb.cpp
	./SchemaItems/SchemaItemValue.cpp
	./SchemaItems/SchemaItemVduImage.cpp
	./SchemaItems/SchemaItemVduLine.cpp
	./SchemaItems/SchemaItemVduRect.cpp
	./SchemaItems/SchemaItemVduValue.cpp

	AppSignalController.cpp
	BaseSchemaWidget.cpp
	ClientSchemaView.cpp
	ClientSchemaWidget.cpp
	Context.cpp
	DiagSchema.cpp
	DiagStateController.cpp
	DrawParam.cpp
	FontParam.cpp
	HorzVertLinks.cpp
	LogController.cpp
	LogicSchema.cpp
	MacrosExpander.cpp
	MonitorSchema.cpp
	PropertyNames.cpp
	Schema.cpp
	SchemaDetails.cpp
	SchemaLayer.cpp
	SchemaManager.cpp
	SchemaPoint.cpp
	SchemaView.cpp
	Session.cpp
	Settings.cpp
	TuningController.cpp
	TuningSchema.cpp
	UfbSchema.cpp
	VduSchema.cpp
	VFrame30Library.cpp
	VFrameTools.cpp
	WiringSchema.cpp
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
		<QJSValueIterator>
		<QRect>
		<QScreen>
		<QScrollArea>
		<QScrollBar>
		<QSlider>
		<QStaticText>
		<QStyleOptionButton>
		<QStyleOptionFrame>
		<QAbstractTextDocumentLayout>
		<QSvgRenderer>
		<QTextDocument>
		<QUuid>
		<QWheelEvent>
		<QWindow>
		<QXmlStreamReader>

		<QtConcurrent/QtConcurrent>

		<CommonLib/Types.h>
		<CommonLib/Hash.h>
		<CommonLib/PropertyObject.h>
		<CommonLib/AfbParamValue.h>

		<ProtoSerialization.h>
		<ProtoCommonHelper.h>
		<VFrame30.pb.h>

		"../../AppSignalLib/AppSignalParam.h"
		"../../AppSignalLib/AppSignalState.h"
		"../../AppSignalLib/TuningValue.h"

		"./include/VFrame30/VFrameTools.h"
)

target_link_libraries(${TARGET} PUBLIC Qt6::Widgets Qt6::Qml Qt6::Xml Qt6::Svg)
target_link_libraries(${TARGET} PUBLIC CommonLib Behavior ProtobufEx UtilsLib AppSignalLib HardwareLib DiagStateLib TrendView)

target_compile_definitions(${TARGET} PRIVATE VFRAME30_LIBRARY)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
