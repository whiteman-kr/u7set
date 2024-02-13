cmake_minimum_required(VERSION 3.16)

project(TuningSrv LANGUAGES CXX)

include(../cmake/vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Widgets Qml Xml)
qt_standard_project_setup()

set(TARGET TuningSrv)

add_executable(${TARGET}
	../lib/DataSource.cpp
	../lib/DataSource.h
	../lib/TuningDataStorage.cpp
	../lib/TuningDataStorage.h
	../Proto/network.proto
	TcpTuningServer.cpp
	TcpTuningServer.h
	TuningClientContext.cpp
	TuningClientContext.h
	TuningMemory.cpp
	TuningMemory.h
	TuningService.cpp
	TuningService.h
	TuningServiceMain.cpp
	TuningSource.cpp
	TuningSource.h
	TuningSourceThread.cpp
	TuningSourceThread.h
	TuningSignal.cpp
	TuningSignal.h
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

	"../Proto/Network.pb.h"
)

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Network Qt6::Widgets Qt6::Qml Qt6::Xml)
target_link_libraries(${TARGET} PRIVATE CommonLib Protobuf UtilsLib AppSignalLib OnlineLib HardwareLib ServiceLib)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()

add_code_coverage_options(${TARGET} CODE_COVERAGE)
