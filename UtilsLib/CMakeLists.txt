cmake_minimum_required(VERSION 3.16)

project(UtilsLib LANGUAGES CXX)

include(../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Xml Widgets LinguistTools)

set(TARGET UtilsLib)

qt_add_library(${TARGET} STATIC
	CUtils.h
	CsvFile.h
	CsvFile.cpp
	ILogFile.h
	LogFile.h
	LogFile.cpp
	Address16.h
	Crc.h
	Crc.cpp
	DomXmlHelper.h
	DomXmlHelper.cpp
	Queue.h
	Queue.cpp
	SimpleMutex.h
	SimpleThread.h
	SimpleThread.cpp
	Ui/UiTools.h
	Ui/UiTools.cpp
	WUtils.h
	XmlHelper.h
	XmlHelper.cpp
	OutputLog.h
	OutputLog.cpp
	CrashExceptionHandler.h
	CrashExceptionHandler.cpp
)

target_precompile_headers(${TARGET} PUBLIC
	<algorithm>
	<array>
	<atomic>
	<cassert>
	<chrono>
	<cmath>
	<cstdint>
	<functional>
	<memory>
	<set>
	<type_traits>
	<vector>

	<QAbstractItemModel>
	<QAction>
	<QApplication>
	<QClipboard>
	<QComboBox>
	<QCoreApplication>
	<QDateTime>
	<QDateTimeEdit>
	<QDebug>
	<QDesktopServices>
	<QDir>
	<QDomDocument>
	<QFile>
	<QFileDialog>
	<QHeaderView>
	<QHostAddress>
	<QKeyEvent>
	<QLineEdit>
	<QMessageBox>
	<QMetaMethod>
	<QMutex>
	<QObject>
	<QOperatingSystemVersion>
	<QProcess>
	<QRegularExpression>
	<QScreen>
	<QScrollBar>
	<QSettings>
	<QShortcut>
	<QStandardPaths>
	<QString>
	<QTableView>
	<QtEndian>
	<QTextStream>
	<QThread>
	<QTimer>
	<QUuid>
	<QVariant>
	<QVBoxLayout>
	<QXmlStreamReader>
	<QXmlStreamWriter>
)

#Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PUBLIC Qt6::Core Qt6::Network Qt6::Xml Qt6::Widgets)
target_link_libraries(${TARGET} PUBLIC CommonLib)

target_compile_definitions(${TARGET} PRIVATE UTILS_LIB_DOMAIN)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)
