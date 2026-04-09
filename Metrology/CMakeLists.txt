cmake_minimum_required(VERSION 3.16)

project(Metrology LANGUAGES CXX)

include(../cmake/translations.cmake)

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
)

target_precompile_headers(${TARGET} PRIVATE
	# C includes, must be before c++ includes
	#
	<cassert>
	<cstdint>

	# C++ includes
	#
	<algorithm>
	<array>
	<atomic>
	<condition_variable>
	<cstddef>
	<cstring>
	<functional>
	<iostream>
	<limits>
	<list>
	<map>
	<memory>
	<mutex>
	<optional>
	<queue>
	<vector>
	<set>
	<thread>
	<type_traits>
	<unordered_map>
	<unordered_set>

	# Qt includes
	#
	<QAbstractSocket>
	<QApplication>
	<QDateTime>
	<QDebug>
	<QDir>
	<QElapsedTimer>
	<QEventLoop>
	<QFile>
	<QFileInfo>
	<QGroupBox>
	<QHash>
	<QHBoxLayout>
	<QHostAddress>
	<QList>
	<QMap>
	<QMetaClassInfo>
	<QMetaEnum>
	<QMetaObject>
	<QMutex>
	<QMutexLocker>
	<QObject>
	<QQueue>
	<QSettings>
	<QString>
	<QStringList>
	<QTextStream>
	<QThread>
	<QTimer>
	<QtGlobal>
	<QtNetwork/QHostAddress>
	<QUdpSocket>
	<QUuid>
	<QVariant>
	<QVBoxLayout>
	<QVector>
	<QXmlStreamReader>
	<QXmlStreamWriter>

	# Project includes
	#
	<CommonLib/Types.h>

	<CommonLib/Hash.h>

	<Network.pb.h>
)

add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Concurrent Qt6::SerialPort Qt6::Network Qt6::Svg Qt6::Sql Qt6::Qml Qt6::Xml Qt6::Charts)
target_link_libraries(${TARGET} PRIVATE CommonLib ProtobufEx AppSignalLib HardwareLib UtilsLib OnlineLib UiLib ClientLib CommonStdLib)

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

