cmake_minimum_required(VERSION 3.16)

project(AppDataSrv LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Widgets Qml Sql Xml)
qt_standard_project_setup()

set(TARGET GatewaySrv)

add_executable(${TARGET}
	AdsGatewayHandler.cpp
	AdsGatewayHandler.h
	AdsGatewayServer.cpp
	AdsGatewayServer.h
	AppDataServiceClient.cpp
	AppDataServiceClient.h
	AppSignalState.cpp
	AppSignalState.h
	Float16.cpp
	Float16.h
	GatewayHandler.cpp
	GatewayHandler.h
	GatewayService.cpp
	GatewayService.h
	GatewayServiceMain.cpp
	IvsImpulseCommThread.cpp
	IvsImpulseCommThread.h
	IvsImpulseDataProtocol.h
	IvsImpulseGatewayHandler.cpp
	IvsImpulseGatewayHandler.h
	ModbusProtocol.cpp
	ModbusProtocol.h
	ModbusSlaveGatewayHandler.cpp
	ModbusSlaveGatewayHandler.h
	ModbusTcpSlaveThread.cpp
	ModbusTcpSlaveThread.h
	ModbusUdpSlaveThread.cpp
	ModbusUdpSlaveThread.h
	TuningGatewayServer.cpp
	TuningGatewayServer.h
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
	<chrono>
	<functional>
	<iostream>
	<limits>
	<list>
	<map>
	<memory>
	<mutex>
	<vector>
	<set>
	<thread>
	<type_traits>
	<unordered_map>

	# Qt includes
	#
	<QAbstractSocket>
	<QApplication>
	<QCommandLineParser>
	<QCoreApplication>
	<QDateTime>
	<QDebug>
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
	<QRegularExpression>
	<QSettings>
	<QString>
	<QStringList>
	<QTextStream>
	<QThread>
	<QTimer>
	<QUdpSocket>
	<QUuid>
	<QVariant>
	<QVector>
	<QXmlStreamReader>
	<QXmlStreamWriter>
	<QtGlobal>
	<QtNetwork/QHostAddress>

	# Project includes
	#
	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/HostAddressPort.h>

	<Network.pb.h>

	"../AppSignalLib/AppSignal.h"

	"../UtilsLib/WUtils.h"
	"../UtilsLib/XmlHelper.h"
)

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Network Qt6::Widgets Qt6::Qml Qt6::Sql Qt6::Xml)
target_link_libraries(${TARGET} PRIVATE CommonLib ProtobufEx UtilsLib AppSignalLib HardwareLib OnlineLib ServiceLib GatewayLib GatewayClientLib)

add_code_coverage_options(${TARGET} CODE_COVERAGE)

add_subdirectory(example)
