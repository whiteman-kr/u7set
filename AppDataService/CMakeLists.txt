cmake_minimum_required(VERSION 3.16)

project(AppDataSrv LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(gRPC CONFIG REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Network Widgets Qml Sql Xml)
qt_standard_project_setup()

set(TARGET AppDataSrv)

add_executable(${TARGET}
	ApertureFile.cpp
	ApertureFile.h
	AppDataReceiver.cpp
	AppDataReceiver.h
	AppDataService.cpp
	AppDataService.h
	AppDataServiceMain.cpp
	AppDataSource.cpp
	AppDataSource.h
	AppDataSrvTools.cpp
	AppDataSrvTools.h
	DiscretesLog.cpp
	DiscretesLog.h
	DynamicAppSignalState.cpp
	DynamicAppSignalState.h
	GrpcAppDataSrv.cpp
	GrpcAppDataSrv.h
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

	<asio/io_context.hpp>
	<asio/ip/udp.hpp>
	<asio/steady_timer.hpp>

	# Qt includes
	#
	<QAbstractSocket>
	<QApplication>
	<QCommandLineParser>
	<QCoreApplication>
	<QDateTime>
	<QDebug>
	<QDir>
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
	<QtSql/QSqlDatabase>
	<QtSql/QSqlError>
	<QtSql/QSqlQuery>
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

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Network Qt6::Widgets Qt6::Qml Qt6::Sql Qt6::Xml)
target_link_libraries(${TARGET} PRIVATE CommonLib ProtobufEx UtilsLib AppSignalLib HardwareLib OnlineLib ServiceLib GrpcLib)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
