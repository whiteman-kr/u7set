cmake_minimum_required(VERSION 3.16)

project(UtilsLib LANGUAGES CXX)

include(../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Xml Widgets LinguistTools)

set(TARGET UtilsLib)

qt_add_library(${TARGET} STATIC
	Address16.h
	CrashExceptionHandler.cpp
	CrashExceptionHandler.h
	Crc.cpp
	Crc.h
	CsvFile.cpp
	CsvFile.h
	CUtils.h
	DomXmlHelper.cpp
	DomXmlHelper.h
	ILogFile.h
	InstanceResolver.cpp
	InstanceResolver.h
	LogFile.cpp
	LogFile.h
	OutputLog.cpp
	OutputLog.h
	Queue.cpp
	Queue.h
	SpinLock.h
	SimpleThread.cpp
	SimpleThread.h
	Ui/UiTools.cpp
	Ui/UiTools.h
	Ui/WidgetUtils.cpp
	Ui/WidgetUtils.h
	WUtils.h
	XmlHelper.cpp
	XmlHelper.h
)

target_precompile_headers(${TARGET} PRIVATE
	<algorithm>
	<array>
	<atomic>
	<cassert>
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
	<QListView>
	<QMessageBox>
	<QMetaMethod>
	<QMutex>
	<QObject>
	<QOperatingSystemVersion>
	<QProcess>
	<QPushButton>
	<QRegularExpression>
	<QScreen>
	<QScrollBar>
	<QSettings>
	<QShortcut>
	<QStandardItem>
	<QStandardItemModel>
	<QStandardPaths>
	<QString>
	<QTableView>
	<QtEndian>
	<QTextStream>
	<QTimer>
	<QUuid>
	<QVariant>
	<QVBoxLayout>
	<QXmlStreamReader>
	<QXmlStreamWriter>
)

target_precompile_headers(${TARGET} PRIVATE
	<CommonLib/Types.h>
	<CommonLib/Hash.h>
)

target_precompile_headers(${TARGET} PUBLIC
	<QThread>
)

#Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PUBLIC Qt6::Core Qt6::Network Qt6::Xml Qt6::Widgets)
target_link_libraries(${TARGET} PUBLIC CommonLib)

target_compile_definitions(${TARGET} PRIVATE UTILS_LIB_DOMAIN)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)
