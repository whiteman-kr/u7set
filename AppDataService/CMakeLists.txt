cmake_minimum_required(VERSION 3.16)

project(AppDataSrv LANGUAGES CXX)

include(../cmake/vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Widgets Qml Sql Xml)
qt_standard_project_setup()

set(TARGET AppDataSrv)

add_executable(${TARGET}
	AppDataReceiver.cpp
	AppDataReceiver.h
	AppDataService.cpp
	AppDataService.h
	AppDataServiceMain.cpp
	AppDataSource.cpp
	AppDataSource.h
	DynamicAppSignalState.cpp
	DynamicAppSignalState.h
	RtTrendsServer.cpp
	RtTrendsServer.h
	SignalStatesProcessingThread.cpp
	SignalStatesProcessingThread.h
	TcpAppDataServer.cpp
	TcpAppDataServer.h
	TcpArchiveClient.cpp
	TcpArchiveClient.h
)

target_precompile_headers(${TARGET} PRIVATE
	# C includes, must be before c++ includes
	#
	<cassert>
	<cstdint>

	# C++ includes
	#
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

	# Qt includes
	#
	<QAbstractSocket>
	<QApplication>
	<QCommandLineParser>
	<QCoreApplication>
	<QDebug>
	<QDateTime>
	<QElapsedTimer>
	<QEventLoop>
	<QFile>
	<QFileInfo>
	<QHash>
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
	<QVector>
	<QXmlStreamReader>
	<QXmlStreamWriter>

	# Project includes
	#
	<CommonLib/Types.h>
	<CommonLib/Hash.h>

	<Network.pb.h>

	"../AppSignalLib/AppSignal.h"
)

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h


target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Network Qt6::Widgets Qt6::Qml Qt6::Sql Qt6::Xml)
target_link_libraries(${TARGET} PRIVATE CommonLib Protobuf UtilsLib AppSignalLib HardwareLib OnlineLib ServiceLib)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()

add_code_coverage_options(${TARGET} CODE_COVERAGE)
