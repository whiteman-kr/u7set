cmake_minimum_required(VERSION 3.25)

project(ArchSrv LANGUAGES CXX)

include(../cmake/vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Widgets Qml Sql Xml)
qt_standard_project_setup()

set(TARGET ArchSrv)
add_executable(${TARGET}
	ArchFile.cpp
	ArchFile.h
	ArchFileBuffer.cpp
	ArchFileBuffer.h
	ArchFileRecord.cpp
	ArchFileRecord.h
	Archive.cpp
	Archive.h
	ArchivingService.cpp
	ArchivingService.h
	ArchMaintenance.cpp
	ArchMaintenance.h
	ArchRequest.cpp
	ArchRequest.h
	ArchServiceMain.cpp
	ArchWriterThread.cpp
	ArchWriterThread.h
	BinSearch.h
	FileArchReader.cpp
	FileArchReader.h
	TcpAppDataServer.cpp
	TcpAppDataServer.h
	TcpArchRequestsServer.cpp
	TcpArchRequestsServer.h
	TimeFilter.cpp
	TimeFilter.h
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

	<CommonLib/Types.h>
	<CommonLib/Hash.h>

	<Network.pb.h>
)

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Network Qt6::Widgets Qt6::Qml Qt6::Sql Qt6::Xml)
target_link_libraries(${TARGET} PRIVATE CommonLib ProtobufEx UtilsLib AppSignalLib OnlineLib ServiceLib)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()

add_code_coverage_options(${TARGET} CODE_COVERAGE)
