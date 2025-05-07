cmake_minimum_required(VERSION 3.16)

project(OnlineLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Network Qml)

set(TARGET OnlineLib)

qt_add_library(${TARGET} STATIC
	AcquiredDiagSignal.cpp
	AcquiredDiagSignal.h
	ApertureRecord.cpp
	ApertureRecord.h
	BuildInfo.cpp
	BuildInfo.h
	CfgLoader.cpp
	CfgLoader.h
	CircularLogger.cpp
	CircularLogger.h
	DataSource.cpp
	DataSource.h
	MatsUsers.cpp
	MatsUsers.h
	OnlineDataSource.cpp
	OnlineDataSource.h
	OnlineDataSources.cpp
	OnlineDataSources.h
	RupFramesReceiver.cpp
	RupFramesReceiver.h
	SignalStatesDistributor.cpp
	SignalStatesDistributor.h
	SignalStatesProcessor.cpp
	SignalStatesProcessor.h
	SoftwareEndpoint.h
	SocketIO.cpp
	SocketIO.h
	SoftwareInfo.cpp
	SoftwareInfo.h
	SoftwareSettings.cpp
	SoftwareSettings.h
	Tcp.cpp
	Tcp.h
	TcpClientStatistics.cpp
	TcpClientStatistics.h
	TcpConnectionState.cpp
	TcpConnectionState.h
	TcpFileTransfer.cpp
	TcpFileTransfer.h
	UdpSocket.cpp
	UdpSocket.h
)

target_precompile_headers(${TARGET} PRIVATE
	# C++ includes
	#
	<algorithm>
	<array>
	<atomic>
	<cassert>
	<cmath>
	<concepts>
	<condition_variable>
	<cstdint>
	<cstdlib>
	<functional>
	<limits>
	<list>
	<map>
	<memory>
	<optional>
	<queue>
	<set>
	<thread>
	<type_traits>
	<unordered_map>
	<utility>
	<vector>

	# Qt includes
	#
	<QAbstractSocket>
	<QCommandLineParser>
	<QCoreApplication>
	<QCryptographicHash>
	<QDateTime>
	<QDebug>
	<QDir>
	<QElapsedTimer>
	<QEventLoop>
	<QFile>
	<QFileInfo>
	<QHash>
	<QHostAddress>
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
	<QSslCertificate>
	<QSslPreSharedKeyAuthenticator>
	<QSslSocket>
	<QStandardPaths>
	<QString>
	<QStringList>
	<QTextStream>
	<QtGlobal>
	<QThread>
	<QTimer>
	<QUdpSocket>
	<QUuid>
	<QVariant>
	<QVector>
	<QXmlStreamReader>
	<QXmlStreamWriter>
)

target_precompile_headers(${TARGET} PRIVATE
	<QHostAddress>

	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/PropertyObject.h>
	<CommonLib/HostAddressPort.h>
	
	<ProtoSerialization.h>
	<Network.pb.h>
)

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

target_link_libraries(${TARGET} PRIVATE Qt6::Network Qt6::Qml asio::asio)
target_link_libraries(${TARGET} PUBLIC CommonLib ProtobufEx UtilsLib HardwareLib)

target_compile_definitions(${TARGET} PRIVATE ONLINE_LIB_DOMAIN)
#target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)
