cmake_minimum_required(VERSION 3.16)

project(ReportLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Xml Qml Network Widgets)

set(TARGET ReportLib)

add_library(${TARGET} STATIC
	Report.cpp
	Report.h
	ReportAppSignalProvider.cpp
	ReportAppSignalProvider.h
	ReportDiagStateProvider.cpp
	ReportDiagStateProvider.h
	ReportGenerator.cpp
	ReportGenerator.h
	ReportObject.cpp
	ReportObject.h
	ReportPrinter.cpp
	ReportPrinter.h
	ReportSchemaView.cpp
	ReportSchemaView.h
	ReportTemplate.cpp
	ReportTemplate.h
	TableExportPrint.cpp
	TableExportPrint.h
)

target_precompile_headers(${TARGET} PRIVATE
	<QXmlStreamReader>
	<QTextCursor>
	<QPdfWriter>
	<QPainter>
	<QPrinter>
	<QPrinterInfo>
	<QFile>
	<QBuffer>
	<QMutex>
	<QTextDocument>
	<QTableView>
	<QAbstractItemModel>
	<QPageSetupDialog>
	<QPrintDialog>
	<QPrinter>
	<QMessageBox>
	<QFileInfo>
	<QSettings>
	<QApplication>
	<QHeaderView>

	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/PropertyObject.h>
	
	<ProtoSerialization.h>
	
	"../AppSignalLib/AppSignal.h"
	"../AppSignalLib/AppSignalParam.h"
	"../AppSignalLib/AppSignalState.h"
)

target_link_libraries(${TARGET} PUBLIC Qt6::Gui Qt6::Qml Qt6::Xml Qt6::Network Qt6::Widgets)
target_link_libraries(${TARGET} PUBLIC CommonLib ProtobufEx UtilsLib OnlineLib AppSignalLib TrendView VFrame30)


if (MSVC)
	target_link_libraries(${TARGET} PUBLIC Advapi32)
endif (MSVC)
if (UNIX)
	target_link_libraries(${TARGET} PUBLIC pam pam_misc)
endif (UNIX)

target_compile_definitions(${TARGET} PRIVATE CLIENT_LIB_DOMAIN)
#target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)
