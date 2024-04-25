cmake_minimum_required(VERSION 3.16)

project(Metrology LANGUAGES CXX)

include(../cmake/vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../lib/Ui)

if (MSVC)
	# MSVC warning suppressions
	add_compile_options(
		/wd4828 # The file contains a character starting at offset 0x10d2 that is illegal in the current source character set (codepage 65001).
	)
endif (MSVC)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Concurrent SerialPort Network Sql Qml Xml Charts)

if (MSVC)
	find_package(Qt6 REQUIRED COMPONENTS AxContainer)
endif (MSVC)

qt_standard_project_setup()

set (METROLOGY_SOURCES
	../lib/PropertyEditor.cpp
	../lib/PropertyEditor.h
	../lib/Ui/DialogAbout.cpp
	../lib/Ui/DialogAbout.h
	Calibrator.cpp
	Calibrator.h
	CalibratorBase.cpp
	CalibratorBase.h
	CalibratorManager.cpp
	CalibratorManager.h
	ChartView.cpp
	ChartView.h
	ConfigSocket.cpp
	ConfigSocket.h
	Database.cpp
	Database.h
	Delegate.cpp
	Delegate.h
	DialogCalculator.cpp
	DialogCalculator.h
	DialogComparatorList.cpp
	DialogComparatorList.h
	DialogList.cpp
	DialogList.h
	DialogMeasurePoint.cpp
	DialogMeasurePoint.h
	DialogMetrologyConnection.cpp
	DialogMetrologyConnection.h
	DialogObjectProperties.cpp
	DialogObjectProperties.h
	DialogOptions.cpp
	DialogOptions.h
	DialogOptionsMvh.cpp
	DialogOptionsMvh.h
	DialogRackList.cpp
	DialogRackList.h
	DialogSignalList.cpp
	DialogSignalList.h
	DialogTuningSignalList.cpp
	DialogTuningSignalList.h
	DialogTuningSourceList.cpp
	DialogTuningSourceList.h
	ExcelHelper.cpp
	ExcelHelper.h
	MeasureBase.cpp
	MeasureBase.h
	MeasurePointBase.cpp
	MeasurePointBase.h
	MeasureThread.cpp
	MeasureThread.h
	MeasureView.cpp
	MeasureView.h
	MeasureViewHeader.cpp
	MeasureViewHeader.h
	MetrologyConnection.cpp
	MetrologyConnection.h
	MetrologyFormula.cpp
	MetrologyFormula.h
	MetrologyMain.cpp
	MetrologyMainWindow.cpp
	MetrologyMainWindow.h
	MetrologySignal.cpp
	MetrologySignal.h
	Options.cpp
	Options.h
	PanelComparatorInfo.cpp
	PanelComparatorInfo.h
	PanelFindMeasure.cpp
	PanelFindMeasure.h
	PanelSignalInfo.cpp
	PanelSignalInfo.h
	PanelStatistics.cpp
	PanelStatistics.h
	ProcessData.cpp
	ProcessData.h
	RackBase.cpp
	RackBase.h
	SelectSignalWidget.cpp
	SelectSignalWidget.h
	SignalBase.cpp
	SignalBase.h
	SignalSocket.cpp
	SignalSocket.h
	StatisticsBase.cpp
	StatisticsBase.h
	TuningSignalBase.cpp
	TuningSignalBase.h
	TuningSocket.cpp
	TuningSocket.h
	UnitsConverter.cpp
	UnitsConverter.h
	UnitsConverterTable.cpp
	UnitsConverterTable.h
	Visa/visa.h
)

set (METROLOGY_RESOURCES
	Resources.qrc
	../Etc/Logo/RadiyLogo.qrc
)

# Add icon
#
if (MSVC)
	list (APPEND METROLOGY_RESOURCES ./WinResources/WinResources.rc)
endif (MSVC)

set (METROLOGY_FORMS
)

set(TARGET Metrology)

add_executable(${TARGET} WIN32
	${METROLOGY_FORMS}
	${METROLOGY_SOURCES}
	${METROLOGY_RESOURCES}
	languages/Metrology_ru.ts
)

target_precompile_headers(${TARGET} PRIVATE
	<cassert>	
	<algorithm>
	<atomic>
	<functional>
	<iostream>
	<limits>
	<list>
	<map>
	<memory>
	<vector>
	<set>
	<type_traits>
	<stdexcept>

	<QAbstractItemModel>
	<QAbstractSocket>
	<QAbstractTableModel>
	<QAction>
	<QApplication>
	<QCheckBox>
	<QCloseEvent>
	<QColorDialog>
	<QComboBox>
	<QCommandLineParser>
	<QCompleter>
	<QCoreApplication>
	<QCoreApplication>
	<QDateTime>
	<QDebug>
	<QElapsedTimer>
	<QErrorMessage>
	<QEventLoop>
	<QFile>
	<QFileDialog>
	<QFileInfo>
	<QGroupBox>
	<QHash>
	<QHBoxLayout>
	<QHeaderView>
	<QHostAddress>
	<QInputDialog>
	<QLabel>
	<QListView>
	<QListWidget>
	<QMainWindow>
	<QMap>
	<QMenu>
	<QMenuBar>
	<QMessageBox>
	<QMetaClassInfo>
	<QMetaEnum>
	<QMetaObject>
	<QMutex>
	<QMutexLocker>
	<QObject>
	<QPainter>
	<QProgressBar>
	<QProgressDialog>
	<QPushButton>
	<QQueue>
	<QRegularExpression>
	<QRegularExpressionValidator>
	<QScreen>
	<QScrollArea>
	<QScrollBar>
	<QSettings>
	<QSplitter>
	<QStatusBar>
	<QString>
	<QStringList>
	<QTableWidget>
	<QTabWidget>
	<QtConcurrent>
	<QTextEdit>
	<QTextStream>
	<QThread>
	<QTimer>
	<QtNetwork/QHostAddress>
	<QTranslator>
	<QTreeWidget>
	<QUdpSocket>
	<QUuid>
	<QVariant>
	<QVBoxLayout>
	<QVector>
	<QXmlStreamReader>
	<QXmlStreamWriter>

	# Project includes
	#
	"../CommonLib/Types.h"
	"../CommonLib/Hash.h"
	"../CommonLib/PropertyObject.h"

	<ProtoSerialization.h>
	<Network.pb.h>
	<AppSignal.pb.h>

	"../AppSignalLib/AppSignal.h"
	"../AppSignalLib/AppSignalState.h"
)

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Concurrent Qt6::SerialPort Qt6::Network Qt6::Sql Qt6::Qml Qt6::Xml Qt6::Charts)
target_link_libraries(${TARGET} PRIVATE CommonLib Protobuf AppSignalLib HardwareLib UtilsLib OnlineLib)

# Interface for calibrator Rigol DG1062Z
#
if (MSVC)
	target_link_libraries(${TARGET} PRIVATE Qt6::AxContainer ${CMAKE_CURRENT_SOURCE_DIR}/Visa/visa64.lib)

	set(DLL_COPY_FILE_LIST
		${CMAKE_CURRENT_SOURCE_DIR}/../bin_Win64/visa64.dll
		${CMAKE_CURRENT_SOURCE_DIR}/../bin_Win64/visaConfMgr.dll
		${CMAKE_CURRENT_SOURCE_DIR}/../bin_Win64/visaUtilities.dll
	)

	add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${DLL_COPY_FILE_LIST} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()


# Link Visual Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()


