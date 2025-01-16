cmake_minimum_required(VERSION 3.16)

project(ReportLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Xml Qml Network Widgets)

set(TARGET ReportLib)

qt_add_library(${TARGET} STATIC
	./include/ReportLib/Report.h
	./include/ReportLib/ReportAppSignalProvider.h
	./include/ReportLib/ReportDiagStateProvider.h
	./include/ReportLib/ReportObject.h
	./include/ReportLib/ReportPrinter.h
	./include/ReportLib/ReportSchemaView.h
	./include/ReportLib/ReportTemplate.h
	./include/ReportLib/TableViewReportGenerator.h
	./include/ReportLib/TaggedReportGenerator.h
	Report.cpp
	ReportAppSignalProvider.cpp
	ReportDiagStateProvider.cpp
	ReportObject.cpp
	ReportPrinter.cpp
	ReportPrinterPrivate.cpp
	ReportPrinterPrivate.h
	ReportSchemaView.cpp
	ReportTemplate.cpp
	TableViewReportGenerator.cpp
	TableViewReportPrivate.h
	TableViewReportPrivate.cpp
	TaggedReportGenerator.cpp
	TaggedReportPrivate.cpp
	TaggedReportPrivate.h
)

target_precompile_headers(${TARGET} PRIVATE
	<QAbstractItemModel>
	<QApplication>
	<QBuffer>
	<QFile>
	<QFileInfo>
	<QFont>
	<QHeaderView>
	<QMutex>
	<QMessageBox>
	<QPdfWriter>
	<QPageLayout>
	<QPainter>
	<QPrinter>
	<QPrinterInfo>
	<QPageSetupDialog>
	<QPrintDialog>
	<QPrinter>
	<QPageLayout>
	<QSettings>
	<QTableView>
	<QTextDocument>
	<QTextCursor>
	<QUuid>
	<QXmlStreamReader>

	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/PropertyObject.h>
	
	<ProtoSerialization.h>
	
	"../../AppSignalLib/AppSignal.h"
	"../../AppSignalLib/AppSignalParam.h"
	"../../AppSignalLib/AppSignalState.h"
)

target_compile_definitions(${TARGET} PRIVATE APPSIGNALLISTS_LIB_DOMAIN)

#Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PUBLIC Qt6::Gui Qt6::Qml Qt6::Xml Qt6::Network Qt6::Widgets)
target_link_libraries(${TARGET} PUBLIC CommonLib ProtobufEx UtilsLib OnlineLib AppSignalLib TrendView VFrame30)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
